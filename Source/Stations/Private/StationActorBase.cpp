#include "StationActorBase.h"
#include "Components/StaticMeshComponent.h"
#include "CarriableComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(MS_StationActorBase, Log, All);

namespace
{
float GetInteractionDefinitionDuration(const UInteractionDefinitionAsset* Definition)
{
	if (Definition == nullptr)
	{
		return 0.0f;
	}

	return Definition->Type == EInteractionType::QTE
		? Definition->QTE.MaxDurationSeconds
		: Definition->IFT.MaxDurationSeconds;
}
}

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
	SetRootComponent(StationMesh);

	ItemHolder = CreateDefaultSubobject<UHolderComponent>(TEXT("ItemHolder"));
	ItemHolder->SetupAttachment(StationMesh);
	ItemHolder->SetIsReplicated(true);
}

void AStationActorBase::BeginPlay()
{
	Super::BeginPlay();

	if (StationMesh && ItemHolder && ItemSocketName != NAME_None)
	{
		// TODO (Nath): Add editor validation tool to detect missing station sockets before runtime.
		if (!StationMesh->DoesSocketExist(ItemSocketName))
		{
			UE_LOGFMT(MS_StationActorBase, Warning, "Item socket '{0}' does not exist on station '{1}'. Falling back to root attachment.", ItemSocketName.ToString(), GetName());
		}
		else
		{
			ItemHolder->AttachToComponent(StationMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ItemSocketName);
		}
	}

	SetStationState(EStationState::Idle);
}

void AStationActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStationActorBase, StationState);
	DOREPLIFETIME(AStationActorBase, CurrentInstruction);
	DOREPLIFETIME(AStationActorBase, ProcessingStartTime);
	DOREPLIFETIME(AStationActorBase, ProcessingDuration);
}

void AStationActorBase::QueueInstruction(const FInstruction& InInstruction)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(MS_StationActorBase, Warning, "QueueInstruction must execute on authority.");
		return;
	}

	InstructionQueue.Add(InInstruction);
	OnInstructionQueuedBP(InInstruction);
}

void AStationActorBase::SetInstruction(const FInstruction& InInstruction)
{
	ClearInstructionQueue();
	QueueInstruction(InInstruction);
}

void AStationActorBase::ClearInstructionQueue()
{
	if (!HasAuthority())
	{
		UE_LOGFMT(MS_StationActorBase, Warning, "ClearInstructionQueue must execute on authority.");
		return;
	}

	InstructionQueue.Reset();
}

void AStationActorBase::SubmitInteractionAttempt(bool bSuccess)
{
	if (StationState != EStationState::Processing || ActiveInteraction == nullptr)
	{
		return;
	}

	ActiveInteraction->RegisterAttempt(bSuccess);
}

TArray<UActivityAsset*> AStationActorBase::GetActivities() const
{
	TArray<UActivityAsset*> Result;
	Result.Reserve(Activities.Num());
	for (UActivityAsset* Activity : Activities)
	{
		Result.Add(Activity);
	}
	return Result;
}

void AStationActorBase::Interact(APlayerController& InInstigator)
{
	if (!HasAuthority() || StationState == EStationState::Processing || ItemHolder == nullptr)
	{
		return;
	}

	APawn* Pawn = InInstigator.GetPawn();
	if (Pawn == nullptr)
	{
		return;
	}

	UHolderComponent* PlayerHolder = Pawn->FindComponentByClass<UHolderComponent>();
	if (PlayerHolder == nullptr)
	{
		UE_LOGFMT(MS_StationActorBase, Warning, "Station interaction failed: Player has no HolderComponent");
		return;
	}

	CurrentInstigator = Pawn;

	UCarriableComponent* PlayerItem = PlayerHolder->GetCarriable();
	UCarriableComponent* StationItem = ItemHolder->GetCarriable();

	if (PlayerItem && !StationItem)
	{
		const FPrimaryAssetId ItemId = PlayerItem->GetItemId();
		if (!CanPlaceItem(ItemId))
		{
			return;
		}

		FInstruction ResolvedInstruction;
		if (!TryResolveInstructionForItem(ItemId, ResolvedInstruction))
		{
			UE_LOGFMT(MS_StationActorBase, Log, "No instruction found for item {0} on station {1}", ItemId.ToString(), GetName());
			return;
		}

		UCarriableComponent* MovedCarriable = PlayerHolder->Replace(nullptr);
		if (MovedCarriable == nullptr)
		{
			return;
		}

		ItemHolder->Replace(MovedCarriable);
		StartProcessing(ResolvedInstruction);
		return;
	}

	if (!PlayerItem && StationItem)
	{
		UCarriableComponent* MovedCarriable = ItemHolder->Replace(nullptr);
		if (MovedCarriable)
		{
			PlayerHolder->Replace(MovedCarriable);
		}

		CurrentInstruction = FInstruction{};
		ProcessingStartTime = 0.0f;
		ProcessingDuration = 0.0f;
		SetStationState(EStationState::Idle);
	}
}

bool AStationActorBase::CanPlaceItem(const FPrimaryAssetId& ItemId) const
{
	// TODO (Nath): Override in derived stations for strict item filters (e.g. only ingredient tags).
	return ItemId.IsValid();
}

bool AStationActorBase::CanExecuteInstruction(const FInstruction& Instruction) const
{
	if (Instruction.Activity.IsNull())
	{
		return true;
	}

	const FSoftObjectPath ActivityPath = Instruction.Activity.ToSoftObjectPath();
	for (const UActivityAsset* SupportedActivity : Activities)
	{
		if (SupportedActivity && FSoftObjectPath(SupportedActivity) == ActivityPath)
		{
			return true;
		}
	}

	return false;
}

void AStationActorBase::StartProcessing(const FInstruction& Instruction)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentInstruction = Instruction;
	ProcessingStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	ProcessingDuration = FMath::Max(0.0f, Instruction.ProcessingDuration);
	SetStationState(EStationState::Processing);
	OnStartProcessingBP(CurrentInstruction);

	ActiveInteraction = nullptr;
	if (const UInteractionDefinitionAsset* InteractionDefinition = ResolveInteractionDefinition(Instruction))
	{
		UInteractionBase* CreatedInteraction = NewObject<UInteractionBase>(this);
		if (CreatedInteraction && CreatedInteraction->StartInteraction(InteractionDefinition))
		{
			ActiveInteraction = CreatedInteraction;
			ActiveInteraction->OnInteractionFinished.AddDynamic(this, &AStationActorBase::HandleInteractionFinished);

			if (ProcessingDuration <= 0.0f)
			{
				ProcessingDuration = GetInteractionDefinitionDuration(InteractionDefinition);
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(ExecutionTickTimer, this, &AStationActorBase::OnExecutionTick, ExecutionTickInterval, true);

		if (ActiveInteraction == nullptr)
		{
			if (ProcessingDuration > 0.0f)
			{
				World->GetTimerManager().SetTimer(ProcessingTimer, this, &AStationActorBase::OnProcessingTimerFinished, ProcessingDuration, false);
			}
			else
			{
				FinishProcessing();
			}
		}
	}
	else
	{
		// TODO (Nath): Add subsystem-level station recovery when world/timer manager is unavailable.
		CancelProcessing();
	}
}

void AStationActorBase::OnProcessingTimerFinished()
{
	if (StationState == EStationState::Processing && ActiveInteraction == nullptr)
	{
		FinishProcessing();
	}
}

void AStationActorBase::OnExecutionTick()
{
	if (StationState != EStationState::Processing)
	{
		StopExecutionTimers();
		return;
	}

	if (CurrentInstruction.bRequiresProximity)
	{
		if (!CurrentInstigator.IsValid())
		{
			CancelProcessing();
			return;
		}

		const float DistanceSquared = FVector::DistSquared(GetActorLocation(), CurrentInstigator->GetActorLocation());
		if (DistanceSquared > FMath::Square(InteractionDistance))
		{
			CancelProcessing();
			return;
		}
	}

	if (ActiveInteraction)
	{
		ActiveInteraction->AdvanceTime(ExecutionTickInterval);
	}
}

void AStationActorBase::HandleInteractionFinished(FInteractionOutput InteractionOutput)
{
	OnInteractionResolvedBP(InteractionOutput);

	if (StationState != EStationState::Processing)
	{
		return;
	}

	if (InteractionOutput.InteractionResult == EInteractionResult::Success)
	{
		FinishProcessing();
		return;
	}

	CancelProcessing();
}

void AStationActorBase::FinishProcessing()
{
	if (!HasAuthority())
	{
		return;
	}

	StopExecutionTimers();

	if (ActiveInteraction)
	{
		ActiveInteraction->OnInteractionFinished.RemoveDynamic(this, &AStationActorBase::HandleInteractionFinished);
		ActiveInteraction = nullptr;
	}

	OnFinishProcessingBP(CurrentInstruction);

	if (ItemHolder)
	{
		UCarriableComponent* InputCarriable = ItemHolder->Replace(nullptr);
		if (InputCarriable && InputCarriable->GetOwner())
		{
			if (AItemActor* InputItemActor = Cast<AItemActor>(InputCarriable->GetOwner()))
			{
				InputItemActor->DestroyItem(true);
			}
			else
			{
				InputCarriable->GetOwner()->Destroy();
			}
		}
	}

	bool bProducedOutput = false;
	if (CurrentInstruction.OutputItem.IsValid() && GetWorld() && ItemHolder)
	{
		UItemAsset* OutputAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(CurrentInstruction.OutputItem);
		if (OutputAsset == nullptr)
		{
			const FSoftObjectPath OutputPath = UAssetManager::Get().GetPrimaryAssetPath(CurrentInstruction.OutputItem);
			OutputAsset = Cast<UItemAsset>(OutputPath.TryLoad());
		}

		if (OutputAsset)
		{
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AItemActor* NewItem = GetWorld()->SpawnActor<AItemActor>(AItemActor::StaticClass(), ItemHolder->GetComponentTransform(), SpawnParameters);
			if (NewItem)
			{
				NewItem->SetItemAsset(OutputAsset);
				if (UCarriableComponent* NewCarriable = NewItem->FindComponentByClass<UCarriableComponent>())
				{
					ItemHolder->Replace(NewCarriable);
					bProducedOutput = true;
				}
			}
		}
		else
		{
			// TODO (Nath): Route missing output asset to a station error event consumed by game flow/debug tools.
			UE_LOGFMT(MS_StationActorBase, Warning, "Failed to load output item asset {0}", CurrentInstruction.OutputItem.ToString());
		}
	}

	CurrentInstruction = FInstruction{};
	ProcessingStartTime = 0.0f;
	ProcessingDuration = 0.0f;
	CurrentInstigator.Reset();

	SetStationState(bProducedOutput ? EStationState::Completed : EStationState::Idle);
}

void AStationActorBase::CancelProcessing()
{
	if (StationState != EStationState::Processing)
	{
		return;
	}

	StopExecutionTimers();

	if (ActiveInteraction)
	{
		ActiveInteraction->OnInteractionFinished.RemoveDynamic(this, &AStationActorBase::HandleInteractionFinished);
		ActiveInteraction = nullptr;
	}

	CurrentInstruction = FInstruction{};
	ProcessingStartTime = 0.0f;
	ProcessingDuration = 0.0f;
	CurrentInstigator.Reset();
	SetStationState(EStationState::Idle);

	// TODO (Nath): Decide station policy on cancellation (re-queue instruction or keep consumed).
	OnCancelProcessingBP();
}

bool AStationActorBase::TryResolveInstructionForItem(const FPrimaryAssetId& ItemId, FInstruction& OutInstruction)
{
	for (int32 Index = 0; Index < InstructionQueue.Num(); ++Index)
	{
		if (InstructionQueue[Index].InputItem == ItemId && CanExecuteInstruction(InstructionQueue[Index]))
		{
			OutInstruction = InstructionQueue[Index];
			InstructionQueue.RemoveAt(Index);
			OnInstructionConsumedBP(OutInstruction);
			return true;
		}
	}

	for (const FInstruction& Instruction : PossibleInstructions)
	{
		if (Instruction.InputItem == ItemId && CanExecuteInstruction(Instruction))
		{
			OutInstruction = Instruction;
			return true;
		}
	}

	return false;
}

const UInteractionDefinitionAsset* AStationActorBase::ResolveInteractionDefinition(const FInstruction& Instruction) const
{
	if (Instruction.Activity.IsNull())
	{
		return nullptr;
	}

	const FSoftObjectPath InstructionActivityPath = Instruction.Activity.ToSoftObjectPath();
	for (const FStationActivityInteraction& ActivityInteraction : ActivityInteractions)
	{
		if (ActivityInteraction.InteractionDefinition == nullptr)
		{
			continue;
		}

		if (ActivityInteraction.Activity.ToSoftObjectPath() == InstructionActivityPath)
		{
			return ActivityInteraction.InteractionDefinition;
		}
	}

	return nullptr;
}

void AStationActorBase::StopExecutionTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProcessingTimer);
		World->GetTimerManager().ClearTimer(ExecutionTickTimer);
	}
}

void AStationActorBase::SetStationState(EStationState NewState)
{
	if (StationState == NewState)
	{
		return;
	}

	StationState = NewState;
	OnStationStateChangedBP(StationState);
}

void AStationActorBase::OnRep_StationState()
{
	OnStationStateChangedBP(StationState);
}
