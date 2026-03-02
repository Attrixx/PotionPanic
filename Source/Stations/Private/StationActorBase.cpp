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

bool AreInstructionsEquivalent(const FInstruction& Left, const FInstruction& Right)
{
	if (Left.InputItem != Right.InputItem
		|| Left.InputQuantity != Right.InputQuantity
		|| Left.OutputItem != Right.OutputItem
		|| Left.OutputQuantity != Right.OutputQuantity
		|| Left.bRequiresProximity != Right.bRequiresProximity
		|| Left.bConsumeInputOnSuccess != Right.bConsumeInputOnSuccess
		|| Left.bProduceOutputOnSuccess != Right.bProduceOutputOnSuccess
		|| Left.bConsumeInputOnFailure != Right.bConsumeInputOnFailure
		|| Left.FailureOutputItem != Right.FailureOutputItem
		|| Left.FailureOutputQuantity != Right.FailureOutputQuantity)
	{
		return false;
	}

	if (!FMath::IsNearlyEqual(Left.ProcessingDuration, Right.ProcessingDuration, KINDA_SMALL_NUMBER))
	{
		return false;
	}

	return Left.Activity.ToSoftObjectPath() == Right.Activity.ToSoftObjectPath();
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

#if WITH_EDITOR
void AStationActorBase::CheckForErrors()
{
	Super::CheckForErrors();

	if (StationMesh == nullptr || ItemSocketName == NAME_None)
	{
		return;
	}

	if (!StationMesh->DoesSocketExist(ItemSocketName))
	{
		UE_LOGFMT(
			MS_StationActorBase,
			Warning,
			"Map check: station '{0}' references missing ItemSocket '{1}'.",
			GetName(),
			ItemSocketName.ToString());
	}
}
#endif

void AStationActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStationActorBase, StationState);
	DOREPLIFETIME(AStationActorBase, CurrentInstruction);
	DOREPLIFETIME(AStationActorBase, ProcessingStartTime);
	DOREPLIFETIME(AStationActorBase, ProcessingDuration);
	DOREPLIFETIME(AStationActorBase, BufferedInputCount);
	DOREPLIFETIME(AStationActorBase, PendingOutputCount);
	DOREPLIFETIME(AStationActorBase, PendingOutputItem);
}

void AStationActorBase::QueueInstruction(const FInstruction& InInstruction)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(MS_StationActorBase, Warning, "QueueInstruction must execute on authority.");
		return;
	}

	FInstruction Instruction = InInstruction;
	Instruction.InputQuantity = FMath::Max(1, Instruction.InputQuantity);
	Instruction.OutputQuantity = FMath::Max(1, Instruction.OutputQuantity);

	InstructionQueue.Add(Instruction);
	OnInstructionQueuedBP(Instruction);
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

	if (TryHandlePlayerItemInteraction(PlayerHolder, PlayerItem, StationItem))
	{
		return;
	}

	if (TryHandleStationItemInteraction(PlayerHolder, PlayerItem, StationItem))
	{
		return;
	}

	if (TryHandlePendingOutputInteraction(PlayerItem, StationItem))
	{
		return;
	}

	if (PlayerItem == nullptr && StationItem == nullptr && BufferedInputCount > 0)
	{
		ResetBufferedBatch();
	}
}

bool AStationActorBase::TryHandlePlayerItemInteraction(UHolderComponent* PlayerHolder, UCarriableComponent* PlayerItem, UCarriableComponent* StationItem)
{
	if (PlayerItem == nullptr)
	{
		return false;
	}

	if (PendingOutputCount > 0 || PendingOutputItem.IsValid())
	{
		return true;
	}

	const FPrimaryAssetId ItemId = PlayerItem->GetItemId();
	if (!CanPlaceItem(ItemId))
	{
		return true;
	}

	FInstruction ResolvedInstruction;
	const bool bHasBufferedBatch =
		BufferedInputCount > 0 &&
		CurrentInstruction.InputItem.IsValid() &&
		BufferedInputCount < GetRequiredInputCount(CurrentInstruction);

	if (bHasBufferedBatch)
	{
		if (CurrentInstruction.InputItem != ItemId || !CanExecuteInstruction(CurrentInstruction))
		{
			return true;
		}

		ResolvedInstruction = CurrentInstruction;
	}
	else if (!TryResolveInstructionForItem(ItemId, ResolvedInstruction))
	{
		UE_LOGFMT(MS_StationActorBase, Log, "No instruction found for item {0} on station {1}", ItemId.ToString(), GetName());
		return true;
	}

	const int32 RequiredInputCount = GetRequiredInputCount(ResolvedInstruction);
	if (RequiredInputCount > 1)
	{
		if (StationItem != nullptr)
		{
			return true;
		}

		if (!ConsumeCarriable(PlayerItem))
		{
			return true;
		}

		CurrentInstruction = ResolvedInstruction;
		BufferedInputCount = FMath::Min(BufferedInputCount + 1, RequiredInputCount);

		if (BufferedInputCount >= RequiredInputCount)
		{
			StopBufferedBatchTimer();
			StartProcessing(CurrentInstruction);
		}
		else if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				BufferedBatchTimer,
				this,
				&AStationActorBase::OnBufferedBatchTimeout,
				FMath::Max(0.1f, BufferedBatchTimeoutSeconds),
				false);
		}
		return true;
	}

	if (StationItem != nullptr)
	{
		return true;
	}

	UCarriableComponent* MovedCarriable = PlayerHolder->Replace(nullptr);
	if (MovedCarriable == nullptr)
	{
		return true;
	}

	ItemHolder->Replace(MovedCarriable);
	CurrentInstruction = ResolvedInstruction;
	BufferedInputCount = 1;
	StartProcessing(ResolvedInstruction);
	return true;
}

bool AStationActorBase::TryHandleStationItemInteraction(UHolderComponent* PlayerHolder, UCarriableComponent* PlayerItem, UCarriableComponent* StationItem)
{
	if (PlayerItem != nullptr || StationItem == nullptr)
	{
		return false;
	}

	UCarriableComponent* MovedCarriable = ItemHolder->Replace(nullptr);
	if (MovedCarriable != nullptr)
	{
		PlayerHolder->Replace(MovedCarriable);
	}

	if (PendingOutputCount > 0)
	{
		TrySpawnPendingOutput();
		SetStationState(ResolveIdleOrCompletedState());
		return true;
	}

	PendingOutputItem = FPrimaryAssetId();
	ResetCurrentInstructionState();
	SetStationState(EStationState::Idle);
	return true;
}

bool AStationActorBase::TryHandlePendingOutputInteraction(UCarriableComponent* PlayerItem, UCarriableComponent* StationItem)
{
	if (PlayerItem != nullptr || StationItem != nullptr || PendingOutputCount <= 0)
	{
		return false;
	}

	TrySpawnPendingOutput();
	SetStationState(ResolveIdleOrCompletedState());
	return true;
}

bool AStationActorBase::CanPlaceItem(const FPrimaryAssetId& ItemId) const
{
	if (!ItemId.IsValid())
	{
		return false;
	}

	if (AllowedInputItems.Num() > 0 && !AllowedInputItems.Contains(ItemId))
	{
		return false;
	}

	const bool bRequiresItemAsset = !bAcceptContainerItems || RequiredInputDataTags.Num() > 0;
	UItemAsset* ItemAsset = nullptr;
	if (bRequiresItemAsset)
	{
		ItemAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(ItemId);
		if (ItemAsset == nullptr)
		{
			const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
			ItemAsset = Cast<UItemAsset>(AssetPath.TryLoad());
		}

		if (ItemAsset == nullptr)
		{
			return false;
		}
	}

	if (!bAcceptContainerItems && ItemAsset != nullptr && ItemAsset->bIsContainer)
	{
		return false;
	}

	if (RequiredInputDataTags.Num() == 0)
	{
		return true;
	}

	int32 MatchedTagCount = 0;
	int32 RequiredTagCount = 0;
	for (const FName RequiredTag : RequiredInputDataTags)
	{
		if (RequiredTag.IsNone())
		{
			continue;
		}

		++RequiredTagCount;
		if (ItemAsset->DataTags.Contains(RequiredTag))
		{
			++MatchedTagCount;
		}
	}

	if (RequiredTagCount <= 0)
	{
		return true;
	}

	return bRequireAllInputDataTags ? (MatchedTagCount >= RequiredTagCount) : (MatchedTagCount > 0);
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

	StopBufferedBatchTimer();
	CurrentInstruction = Instruction;
	RemoveFirstMatchingQueuedInstruction(CurrentInstruction);
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
		ReportRuntimeError(
			EStationRuntimeError::MissingWorldContext,
			FText::FromString(TEXT("Cannot start processing because world context is unavailable.")));
		ResetCurrentInstructionState();
		SetStationState(EStationState::Idle);
		OnCancelProcessingBP();
	}
}

void AStationActorBase::OnProcessingTimerFinished()
{
	if (StationState == EStationState::Processing && ActiveInteraction == nullptr)
	{
		FinishProcessing();
	}
}

void AStationActorBase::OnBufferedBatchTimeout()
{
	if (!HasAuthority() || StationState == EStationState::Processing)
	{
		return;
	}

	ResetBufferedBatch();
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
	const FInstruction CompletedInstruction = CurrentInstruction;

	if (ActiveInteraction)
	{
		ActiveInteraction->OnInteractionFinished.RemoveDynamic(this, &AStationActorBase::HandleInteractionFinished);
		ActiveInteraction = nullptr;
	}

	OnFinishProcessingBP(CompletedInstruction);

	if (CompletedInstruction.bConsumeInputOnSuccess && ItemHolder)
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

	PendingOutputCount = 0;
	PendingOutputItem = FPrimaryAssetId();

	const int32 RequiredOutputCount = GetRequiredOutputCount(CompletedInstruction);
	if (CompletedInstruction.bProduceOutputOnSuccess && CompletedInstruction.OutputItem.IsValid() && RequiredOutputCount > 0)
	{
		PendingOutputItem = CompletedInstruction.OutputItem;
		PendingOutputCount = RequiredOutputCount;
		TrySpawnPendingOutput();
	}

	ResetCurrentInstructionState();
	SetStationState(ResolveIdleOrCompletedState());
	OnInstructionProcessed.Broadcast(this, CompletedInstruction, true);
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

	const FInstruction FailedInstruction = CurrentInstruction;
	const bool bShouldConsumeInputOnFailure = FailedInstruction.bConsumeInputOnFailure;
	if (bShouldConsumeInputOnFailure && ItemHolder != nullptr)
	{
		if (UCarriableComponent* HeldCarriable = ItemHolder->GetCarriable())
		{
			ConsumeCarriable(HeldCarriable);
		}
	}

	PendingOutputCount = 0;
	PendingOutputItem = FPrimaryAssetId();
	bool bHasFailureOutputAvailable = false;
	if (FailedInstruction.FailureOutputItem.IsValid() && FailedInstruction.FailureOutputQuantity > 0)
	{
		PendingOutputItem = FailedInstruction.FailureOutputItem;
		PendingOutputCount = FMath::Max(1, FailedInstruction.FailureOutputQuantity);
		TrySpawnPendingOutput();
		bHasFailureOutputAvailable = ResolveIdleOrCompletedState() == EStationState::Completed;
	}

	ResetCurrentInstructionState();
	SetStationState(bHasFailureOutputAvailable ? EStationState::Completed : EStationState::Idle);

	if (CancelPolicy == EStationCancelPolicy::RequeueInstruction && FailedInstruction.InputItem.IsValid())
	{
		InstructionQueue.Insert(FailedInstruction, 0);
		OnInstructionQueuedBP(FailedInstruction);
	}

	OnInstructionProcessed.Broadcast(this, FailedInstruction, false);
	OnCancelProcessingBP();
}

void AStationActorBase::ResetBufferedBatch()
{
	if (!HasAuthority() || StationState == EStationState::Processing || BufferedInputCount <= 0)
	{
		return;
	}

	const int32 DiscardedCount = BufferedInputCount;
	StopBufferedBatchTimer();
	ResetCurrentInstructionState();
	SetStationState(EStationState::Idle);
	OnBufferedBatchResetBP(DiscardedCount);
}

bool AStationActorBase::ApplyFailureOutcome(const FPrimaryAssetId& FailureOutputItem, int32 FailureOutputQuantity, bool bConsumeHeldItem, bool bClearInstructionQueue)
{
	if (!HasAuthority() || ItemHolder == nullptr)
	{
		return false;
	}

	if (StationState == EStationState::Processing)
	{
		CancelProcessing();
	}

	if (bClearInstructionQueue)
	{
		ClearInstructionQueue();
	}

	if (bConsumeHeldItem)
	{
		if (UCarriableComponent* HeldCarriable = ItemHolder->GetCarriable())
		{
			ConsumeCarriable(HeldCarriable);
		}
	}

	PendingOutputCount = 0;
	PendingOutputItem = FPrimaryAssetId();
	if (FailureOutputItem.IsValid() && FailureOutputQuantity > 0)
	{
		PendingOutputItem = FailureOutputItem;
		PendingOutputCount = FMath::Max(1, FailureOutputQuantity);
		TrySpawnPendingOutput();
	}

	ResetCurrentInstructionState();
	SetStationState(ResolveIdleOrCompletedState());
	return true;
}

bool AStationActorBase::TryResolveInstructionForItem(const FPrimaryAssetId& ItemId, FInstruction& OutInstruction)
{
	for (int32 Index = 0; Index < InstructionQueue.Num(); ++Index)
	{
		if (InstructionQueue[Index].InputItem == ItemId && CanExecuteInstruction(InstructionQueue[Index]))
		{
			OutInstruction = InstructionQueue[Index];
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

bool AStationActorBase::ConsumeCarriable(UCarriableComponent* Carriable) const
{
	if (Carriable == nullptr)
	{
		return false;
	}

	UHolderComponent* SourceHolder = Carriable->GetHolder();
	if (SourceHolder == nullptr)
	{
		return false;
	}

	UCarriableComponent* RemovedCarriable = SourceHolder->Replace(nullptr);
	if (RemovedCarriable == nullptr)
	{
		return false;
	}

	if (AItemActor* ItemActor = Cast<AItemActor>(RemovedCarriable->GetOwner()))
	{
		ItemActor->DestroyItem(true);
		return true;
	}

	if (AActor* OwnerActor = RemovedCarriable->GetOwner())
	{
		OwnerActor->Destroy();
		return true;
	}

	return false;
}

bool AStationActorBase::RemoveFirstMatchingQueuedInstruction(const FInstruction& Instruction)
{
	for (int32 Index = 0; Index < InstructionQueue.Num(); ++Index)
	{
		if (!AreInstructionsEquivalent(InstructionQueue[Index], Instruction))
		{
			continue;
		}

		const FInstruction ConsumedInstruction = InstructionQueue[Index];
		InstructionQueue.RemoveAt(Index);
		OnInstructionConsumedBP(ConsumedInstruction);
		return true;
	}

	return false;
}

bool AStationActorBase::SpawnInstructionOutput(const FInstruction& Instruction)
{
	if (!Instruction.OutputItem.IsValid() || GetWorld() == nullptr || ItemHolder == nullptr || ItemHolder->GetCarriable() != nullptr)
	{
		return false;
	}

	UItemAsset* OutputAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(Instruction.OutputItem);
	if (OutputAsset == nullptr)
	{
		const FSoftObjectPath OutputPath = UAssetManager::Get().GetPrimaryAssetPath(Instruction.OutputItem);
		OutputAsset = Cast<UItemAsset>(OutputPath.TryLoad());
	}

	if (OutputAsset == nullptr)
	{
		const FText ErrorMessage = FText::Format(
			FText::FromString(TEXT("Failed to load output item asset '{0}'.")),
			FText::FromString(Instruction.OutputItem.ToString()));
		UE_LOGFMT(MS_StationActorBase, Warning, "Failed to load output item asset {0}", Instruction.OutputItem.ToString());
		ReportRuntimeError(EStationRuntimeError::MissingOutputAsset, ErrorMessage);
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AItemActor* NewItem = GetWorld()->SpawnActor<AItemActor>(AItemActor::StaticClass(), ItemHolder->GetComponentTransform(), SpawnParameters);
	if (NewItem == nullptr)
	{
		ReportRuntimeError(
			EStationRuntimeError::OutputSpawnFailed,
			FText::FromString(TEXT("Failed to spawn station output actor.")));
		return false;
	}

	NewItem->SetItemAsset(OutputAsset);
	if (UCarriableComponent* NewCarriable = NewItem->FindComponentByClass<UCarriableComponent>())
	{
		ItemHolder->Replace(NewCarriable);
		return true;
	}

	NewItem->DestroyItem(true);
	ReportRuntimeError(
		EStationRuntimeError::OutputSpawnFailed,
		FText::FromString(TEXT("Spawned output item has no carriable component.")));
	return false;
}

void AStationActorBase::TrySpawnPendingOutput()
{
	if (!HasAuthority() || PendingOutputCount <= 0 || !PendingOutputItem.IsValid() || ItemHolder == nullptr || ItemHolder->GetCarriable() != nullptr)
	{
		return;
	}

	FInstruction OutputInstruction;
	OutputInstruction.OutputItem = PendingOutputItem;

	if (!SpawnInstructionOutput(OutputInstruction))
	{
		if (PendingOutputRetryCount < FMath::Max(0, MaxPendingOutputSpawnRetries))
		{
			++PendingOutputRetryCount;

			if (UWorld* World = GetWorld())
			{
				World->GetTimerManager().SetTimer(
					PendingOutputRetryTimer,
					this,
					&AStationActorBase::TrySpawnPendingOutput,
					FMath::Max(0.0f, PendingOutputRetryDelaySeconds),
					false);
			}
		}
		else
		{
			ReportRuntimeError(
				EStationRuntimeError::OutputSpawnFailed,
				FText::FromString(TEXT("Pending output spawn failed after max retries.")));
		}
		return;
	}

	StopPendingOutputRetryTimer();
	PendingOutputRetryCount = 0;
	--PendingOutputCount;
	if (PendingOutputCount <= 0)
	{
		PendingOutputCount = 0;
		PendingOutputItem = FPrimaryAssetId();
	}
}

void AStationActorBase::ResetCurrentInstructionState()
{
	StopBufferedBatchTimer();
	StopPendingOutputRetryTimer();
	CurrentInstruction = FInstruction{};
	ProcessingStartTime = 0.0f;
	ProcessingDuration = 0.0f;
	BufferedInputCount = 0;
	PendingOutputRetryCount = 0;
	CurrentInstigator.Reset();
}

int32 AStationActorBase::GetRequiredInputCount(const FInstruction& Instruction) const
{
	return FMath::Max(1, Instruction.InputQuantity);
}

int32 AStationActorBase::GetRequiredOutputCount(const FInstruction& Instruction) const
{
	return FMath::Max(1, Instruction.OutputQuantity);
}

AItemActor* AStationActorBase::GetHeldItemActor() const
{
	if (ItemHolder == nullptr)
	{
		return nullptr;
	}

	UCarriableComponent* HeldCarriable = ItemHolder->GetCarriable();
	if (HeldCarriable == nullptr)
	{
		return nullptr;
	}

	return Cast<AItemActor>(HeldCarriable->GetOwner());
}

bool AStationActorBase::TryStartProcessingHeldItem(APawn* InstigatorPawn)
{
	if (!HasAuthority() || StationState == EStationState::Processing || ItemHolder == nullptr)
	{
		return false;
	}

	UCarriableComponent* HeldCarriable = ItemHolder->GetCarriable();
	if (HeldCarriable == nullptr)
	{
		return false;
	}

	FInstruction ResolvedInstruction;
	if (!TryResolveInstructionForItem(HeldCarriable->GetItemId(), ResolvedInstruction))
	{
		return false;
	}

	if (GetRequiredInputCount(ResolvedInstruction) > 1)
	{
		return false;
	}

	CurrentInstruction = ResolvedInstruction;
	BufferedInputCount = 1;
	CurrentInstigator = InstigatorPawn;
	StartProcessing(ResolvedInstruction);
	return true;
}

void AStationActorBase::StopExecutionTimers()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ProcessingTimer);
		World->GetTimerManager().ClearTimer(ExecutionTickTimer);
	}
}

void AStationActorBase::StopBufferedBatchTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BufferedBatchTimer);
	}
}

void AStationActorBase::StopPendingOutputRetryTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PendingOutputRetryTimer);
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

EStationState AStationActorBase::ResolveIdleOrCompletedState() const
{
	if (PendingOutputCount > 0)
	{
		return EStationState::Completed;
	}

	if (ItemHolder != nullptr && ItemHolder->GetCarriable() != nullptr)
	{
		return EStationState::Completed;
	}

	return EStationState::Idle;
}

void AStationActorBase::ReportRuntimeError(EStationRuntimeError ErrorCode, const FText& Message)
{
	if (ErrorCode == EStationRuntimeError::None)
	{
		return;
	}

	UE_LOGFMT(
		MS_StationActorBase,
		Warning,
		"Station runtime error on '{0}': {1}",
		GetName(),
		Message.ToString());
	OnStationRuntimeErrorBP(ErrorCode, Message);
}

void AStationActorBase::OnRep_StationState()
{
	OnStationStateChangedBP(StationState);
}
