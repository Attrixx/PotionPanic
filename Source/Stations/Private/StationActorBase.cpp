#include "StationActorBase.h"
#include "HolderComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "ItemAsset.h"
#include "CarriableComponent.h"
#include "ItemProvider.h" // Interface
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/AssetManager.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(LogStations, Log, All);

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	// TODO (Nath): Setup socket component attachment (e.g. "ItemSlot")
	StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
	SetRootComponent(StationMesh);

	ItemHolder = CreateDefaultSubobject<UHolderComponent>(TEXT("Item Holder"));
	ItemHolder->SetupAttachment(StationMesh);
	ItemHolder->SetIsReplicated(true);
}

void AStationActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AStationActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStationActorBase, StationState);
	DOREPLIFETIME(AStationActorBase, ProcessingStartTime);
	DOREPLIFETIME(AStationActorBase, ProcessingDuration);
}

void AStationActorBase::Interact(APlayerController& InInstigator)
{
	if (!InInstigator.GetPawn())
	{
		return;
	}

	APawn* Pawn = InInstigator.GetPawn();
	UHolderComponent* PlayerHolder = Pawn->FindComponentByClass<UHolderComponent>();

	if (!PlayerHolder)
	{
		UE_LOGFMT(LogStations, Warning, "Station interaction failed: Player has no HolderComponent");
		return;
	}

	CurrentInstigator = Pawn;

	UCarriableComponent* PlayerItem = PlayerHolder->GetCarriable();
	UCarriableComponent* StationItem = ItemHolder->GetCarriable();

	if (PlayerItem && !StationItem)
	{
		AActor* ItemActor = PlayerItem->GetOwner();
		// Interface check using IItemProvider
		if (ItemActor && ItemActor->Implements<UItemProvider>())
		{
			UItemAsset* ItemAsset = IItemProvider::Execute_GetItemAsset(ItemActor);
			if (CanPlaceItem(ItemAsset))
			{
				bool bFoundInstruction = false;
				FInstruction FoundInstruction;

				for (const FInstruction& Instr : PossibleInstructions)
				{
					if (Instr.InputItem == ItemAsset->GetPrimaryAssetId() && CanExecuteInstruction(Instr))
					{
						FoundInstruction = Instr;
						bFoundInstruction = true;
						break;
					}
				}

				if (bFoundInstruction)
				{
					// Move item to station
					UCarriableComponent* Old = PlayerHolder->Replace(nullptr);
					ItemHolder->Replace(Old);
					
					StartProcessing(FoundInstruction);
				}
				else
				{
					UE_LOGFMT(LogStations, Log, "No instruction found for item {0} on this station", ItemActor->GetName());
				}
			}
		}
	}
	else if (StationItem && !PlayerItem)
	{
		if (StationState != EStationState::Processing)
		{
			UCarriableComponent* Old = ItemHolder->Replace(nullptr);
			PlayerHolder->Replace(Old);

			// Reset state if needed
			StationState = EStationState::Idle;
		}
	}
}

bool AStationActorBase::CanPlaceItem(const UItemAsset* Item) const
{
	if (!Item) return false;
	return true;
}

bool AStationActorBase::CanExecuteInstruction(const FInstruction& Instruction) const
{
	// 1. Check if Activity is supported by this station
	// 1. Check if Activity is supported by this station
	// Soft Reference Check: Compare Paths
	if (!Instruction.Activity.IsNull())
	{
		bool bIsSupported = false;
		for (UActivityAsset* SupportedActivity : Activities)
		{
			if (SupportedActivity && Instruction.Activity.ToSoftObjectPath() == FSoftObjectPath(SupportedActivity))
			{
				bIsSupported = true;
				break;
			}
		}

		if (!bIsSupported)
		{
			return false;
		}
	}

	return true;
}

void AStationActorBase::StartProcessing(const FInstruction& Instruction)
{
	CurrentInstruction = Instruction;
	StationState = EStationState::Processing;
	OnStationStateChangedBP(StationState); // Update Server Visuals
	ProcessingDuration = Instruction.ProcessingDuration;
	ProcessingStartTime = GetWorld()->GetTimeSeconds();

	if (ProcessingDuration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(ProcessingTimer, this, &AStationActorBase::OnProcessingTimerFinished, ProcessingDuration, false);
		
		OnStartProcessingBP(CurrentInstruction);
		
		if (CurrentInstruction.bRequiresProximity)
		{
		GetWorld()->GetTimerManager().SetTimer(ProximityTimerHandle, this, &AStationActorBase::CheckProximity, 0.1f, true);
		}
	}
	else
	{
		// Instant
		FinishProcessing();
	}
}

void AStationActorBase::CancelProcessing()
{
	if (StationState != EStationState::Processing)
	{
		return;
	}

	StationState = EStationState::Idle;
	OnStationStateChangedBP(StationState); // Update Server Visuals

	GetWorld()->GetTimerManager().ClearTimer(ProcessingTimer);
	GetWorld()->GetTimerManager().ClearTimer(ProximityTimerHandle);
	CurrentInstigator.Reset(); // Or keep it?
	
	OnCancelProcessingBP();
	
	OnCancelProcessingBP();
	
	UE_LOGFMT(LogStations, Log, "Processing cancelled - Player moved too far.");
}

void AStationActorBase::CheckProximity()
{
	if (StationState == EStationState::Processing && CurrentInstruction.bRequiresProximity)
	{
		if (CurrentInstigator.IsValid())
		{
			float DistSq = FVector::DistSquared(GetActorLocation(), CurrentInstigator->GetActorLocation());
			if (DistSq > FMath::Square(InteractionDistance))
			{
				CancelProcessing();
			}
		}
		else
		{
			// Instigator lost (destroyed etc)
			CancelProcessing();
		}
	}
}

void AStationActorBase::OnProcessingTimerFinished()
{
	FinishProcessing();
}

void AStationActorBase::FinishProcessing()
{
	StationState = EStationState::Completed;
	OnStationStateChangedBP(StationState); // Update Server Visuals
	GetWorld()->GetTimerManager().ClearTimer(ProximityTimerHandle);
	
	OnFinishProcessingBP(CurrentInstruction);

	// Execute Output
	// 1. Destroy Input
	UCarriableComponent* InputCarriable = ItemHolder->Replace(nullptr); // Clear ref first
	if (InputCarriable)
	{
		InputCarriable->GetOwner()->Destroy();
	}

	// 2. Spawn Output
	if (CurrentInstruction.OutputItem.IsValid())
	{
		UItemAsset* OutputAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(CurrentInstruction.OutputItem);
		if (OutputAsset)
		{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		// Spawn generic ItemActor and configure it
		AItemActor* NewItem = GetWorld()->SpawnActor<AItemActor>(AItemActor::StaticClass(), GetActorTransform(), SpawnParams);
		if (NewItem)
		{
			NewItem->SetItemAsset(OutputAsset);
			
			// Place in Holder
			UCarriableComponent* NewCarriable = NewItem->FindComponentByClass<UCarriableComponent>();
			if (NewCarriable)
			{
				ItemHolder->Replace(NewCarriable);
			}
		}
		}
	}
	else
	{
		// Output is null (Trash)
		// Do nothing (Input already destroyed)
		StationState = EStationState::Idle; // Reset to idle for trash
	}
}

void AStationActorBase::OnRep_StationState()
{
	// Called on Client when State updates
	OnStationStateChangedBP(StationState);
}
