#include "StationActorBase.h"
#include "HolderComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include "CarriableComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/AssetManager.h"

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = true;
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
		UE_LOG(LogTemp, Warning, TEXT("Station interaction failed: Player has no HolderComponent"));
		return;
	}

	CurrentInstigator = Pawn;

	UCarriableComponent* PlayerItem = PlayerHolder->GetCarriable();
	UCarriableComponent* StationItem = ItemHolder->GetCarriable();

	if (PlayerItem && !StationItem)
	{
		AItemActor* ItemActor = Cast<AItemActor>(PlayerItem->GetOwner());
		if (ItemActor && CanPlaceItem(ItemActor->GetItemAsset()))
		{
			bool bFoundInstruction = false;
			FInstruction FoundInstruction;

			for (const FInstruction& Instr : PossibleInstructions)
			{
				if (Instr.InputItem == ItemActor->GetItemAsset()->GetPrimaryAssetId() && CanExecuteInstruction(Instr))
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
				UE_LOG(LogTemp, Log, TEXT("No instruction found for item %s on this station"), *ItemActor->GetName());
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
	if (Instruction.Activity && !Activities.Contains(Instruction.Activity))
	{
		return false;
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
			SetActorTickEnabled(true);
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
	SetActorTickEnabled(false);
	CurrentInstigator.Reset(); // Or keep it?
	
	OnCancelProcessingBP();
	
	UE_LOG(LogTemp, Log, TEXT("Processing cancelled - Player moved too far."));
}

void AStationActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	SetActorTickEnabled(false);
	
	OnFinishProcessingBP(CurrentInstruction);

	// Execute Output
	// 1. Destroy Input
	UCarriableComponent* InputCarriable = ItemHolder->GetCarriable();
	if (InputCarriable)
	{
		InputCarriable->GetOwner()->Destroy();
		ItemHolder->Replace(nullptr); // Clear ref
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
