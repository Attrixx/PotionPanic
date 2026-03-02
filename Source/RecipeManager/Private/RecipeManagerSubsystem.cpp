// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeManagerSubsystem.h"
#include "CarriableComponent.h"
#include "Instruction.h"
#include "Cauldron.h"
#include "FireStation.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include "RecipeSystem.h"
#include "StationActorBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RecipeManagerSubsystem, Log, All);

void URecipeManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	RefreshFireStations();
	ActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(this, &URecipeManagerSubsystem::HandleActorSpawned));
}

void URecipeManagerSubsystem::Deinitialize()
{
	if (GetWorld() != nullptr && ActorSpawnedHandle.IsValid())
	{
		GetWorld()->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
		ActorSpawnedHandle.Reset();
	}

	for (const TWeakObjectPtr<AFireStation>& WeakStation : BoundFireStations)
	{
		if (AFireStation* FireStation = WeakStation.Get())
		{
			FireStation->OnProcessRequested.RemoveDynamic(this, &URecipeManagerSubsystem::HandleFireStationProcessRequested);
			FireStation->OnDestroyed.RemoveDynamic(this, &URecipeManagerSubsystem::HandleFireStationDestroyed);
			FireStation->OnInstructionProcessed.RemoveAll(this);
		}
	}
	BoundFireStations.Reset();
	ActiveFireStationExecutions.Reset();

	Super::Deinitialize();
}

namespace
{
FInstruction BuildStationInstruction(const FRecipeInstructionData& RecipeInstruction, const URecipeDataAsset* Recipe)
{
	FInstruction StationInstruction;
	StationInstruction.InputItem = RecipeInstruction.InputItem;
	StationInstruction.InputQuantity = FMath::Max(1, RecipeInstruction.InputQuantity);
	StationInstruction.OutputItem = RecipeInstruction.OutputItem;
	StationInstruction.OutputQuantity = FMath::Max(1, RecipeInstruction.OutputQuantity);
	StationInstruction.Activity = RecipeInstruction.Activity;
	StationInstruction.ProcessingDuration = RecipeInstruction.ProcessingDuration;
	StationInstruction.bRequiresProximity = RecipeInstruction.bRequiresProximity;
	StationInstruction.bConsumeInputOnSuccess = true;
	StationInstruction.bProduceOutputOnSuccess = true;

	if (Recipe != nullptr)
	{
		StationInstruction.bConsumeInputOnFailure = Recipe->bConsumeMatchedInputsOnFailure;
		if (Recipe->FailureOutputItem != nullptr)
		{
			StationInstruction.FailureOutputItem = Recipe->FailureOutputItem->GetPrimaryAssetId();
			StationInstruction.FailureOutputQuantity = FMath::Max(1, Recipe->FailureOutputQuantity);
		}
	}

	return StationInstruction;
}
}

FRecipeManagerResult URecipeManagerSubsystem::BuildPlanAndQueueInstructions(
	AStationActorBase* Station,
	const TArray<URecipeDataAsset*>& CandidateRecipes,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueue)
{
	FRecipeValidationResult EmptyValidation;

	if (Station == nullptr)
	{
		return MakeFailure(
			ERecipeManagerError::NullStation,
			FText::FromString(TEXT("Station is null.")),
			nullptr,
			EmptyValidation,
			0);
	}

	if (!IsAuthorityWorld() || !Station->HasAuthority())
	{
		return MakeFailure(
			ERecipeManagerError::NotAuthority,
			FText::FromString(TEXT("BuildPlanAndQueueInstructions must run on authority.")),
			nullptr,
			EmptyValidation,
			0);
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeManagerError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	FRecipeExecutionPlan ExecutionPlan;
	if (!RecipeSystem->TryBuildExecutionPlan(CandidateRecipes, InputItems, ExecutionPlan))
	{
		FRecipeManagerResult FailureResult = MakeFailure(
			ERecipeManagerError::NoMatchingRecipe,
			ExecutionPlan.Validation.FailureReason.IsEmpty()
				? FText::FromString(TEXT("No matching recipe for provided input items."))
				: ExecutionPlan.Validation.FailureReason,
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);

		if (ExecutionPlan.Recipe != nullptr)
		{
			FailureResult.FailureOutcome = RecipeSystem->ResolveFailureOutcome(ExecutionPlan.Recipe.Get(), ExecutionPlan.Validation);
			MaterializeFailureOutcomeOnStation(Station, FailureResult.FailureOutcome, bClearExistingQueue);
		}

		return FailureResult;
	}

	return QueueExecutionPlan(Station, ExecutionPlan, bClearExistingQueue);
}

FRecipeManagerResult URecipeManagerSubsystem::BuildPlanAndQueueFromActiveRoundRecipes(
	AStationActorBase* Station,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueue)
{
	FRecipeValidationResult EmptyValidation;

	if (Station == nullptr)
	{
		return MakeFailure(
			ERecipeManagerError::NullStation,
			FText::FromString(TEXT("Station is null.")),
			nullptr,
			EmptyValidation,
			0);
	}

	if (!IsAuthorityWorld() || !Station->HasAuthority())
	{
		return MakeFailure(
			ERecipeManagerError::NotAuthority,
			FText::FromString(TEXT("BuildPlanAndQueueFromActiveRoundRecipes must run on authority.")),
			nullptr,
			EmptyValidation,
			0);
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeManagerError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	const TArray<URecipeDataAsset*> ActiveRoundRecipes = RecipeSystem->GetActiveRoundRecipes();
	if (ActiveRoundRecipes.Num() == 0)
	{
		return MakeFailure(
			ERecipeManagerError::EmptyActiveRoundRecipes,
			FText::FromString(TEXT("No active round recipes are configured in RecipeSystem.")),
			nullptr,
			EmptyValidation,
			0);
	}

	return BuildPlanAndQueueInstructions(Station, ActiveRoundRecipes, InputItems, bClearExistingQueue);
}

FRecipeManagerResult URecipeManagerSubsystem::QueueExecutionPlan(
	AStationActorBase* Station,
	const FRecipeExecutionPlan& ExecutionPlan,
	bool bClearExistingQueue)
{
	FRecipeValidationResult EmptyValidation;

	if (Station == nullptr)
	{
		return MakeFailure(
			ERecipeManagerError::NullStation,
			FText::FromString(TEXT("Station is null.")),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	if (!Station->HasAuthority())
	{
		return MakeFailure(
			ERecipeManagerError::NotAuthority,
			FText::FromString(TEXT("QueueExecutionPlan must run on authority.")),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	if (ExecutionPlan.Recipe == nullptr || !ExecutionPlan.Validation.bIsValid)
	{
		return MakeFailure(
			ERecipeManagerError::InvalidExecutionPlan,
			FText::FromString(TEXT("Execution plan is invalid or unresolved.")),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	TArray<FInstruction> StationInstructions;
	StationInstructions.Reserve(ExecutionPlan.Instructions.Num());
	for (int32 StepIndex = 0; StepIndex < ExecutionPlan.Instructions.Num(); ++StepIndex)
	{
		const FRecipeInstructionData& RecipeInstruction = ExecutionPlan.Instructions[StepIndex];

		if (!RecipeInstruction.InputItem.IsValid() || !RecipeInstruction.OutputItem.IsValid())
		{
			return MakeFailure(
				ERecipeManagerError::InvalidInstruction,
				FText::Format(
					FText::FromString(TEXT("Instruction at step {0} has invalid input/output item id.")),
					StepIndex),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		const FInstruction StationInstruction = BuildStationInstruction(RecipeInstruction, ExecutionPlan.Recipe.Get());
		if (!Station->CanExecuteInstruction(StationInstruction))
		{
			return MakeFailure(
				ERecipeManagerError::StationCannotExecuteInstruction,
				FText::Format(
					FText::FromString(TEXT("Station cannot execute instruction at step {0} (activity mismatch).")),
					StepIndex),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		StationInstructions.Add(StationInstruction);
	}

	if (bClearExistingQueue)
	{
		Station->ClearInstructionQueue();
	}

	int32 QueuedInstructions = 0;
	for (const FInstruction& StationInstruction : StationInstructions)
	{
		Station->QueueInstruction(StationInstruction);
		++QueuedInstructions;
	}

	FRecipeManagerResult Result;
	Result.bSuccess = true;
	Result.ErrorCode = ERecipeManagerError::None;
	Result.Message = FText::Format(
		FText::FromString(TEXT("Queued {0} instruction(s) on station '{1}'.")),
		QueuedInstructions,
		FText::FromString(Station->GetName()));
	Result.InstructionsQueued = QueuedInstructions;
	Result.ResolvedRecipe = ExecutionPlan.Recipe;
	Result.Validation = ExecutionPlan.Validation;
	return Result;
}

FRecipeManagerResult URecipeManagerSubsystem::BuildPlanAndQueueAcrossStationsFromActiveRoundRecipes(
	const TArray<AStationActorBase*>& StepStations,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueues)
{
	FRecipeValidationResult EmptyValidation;

	if (!IsAuthorityWorld())
	{
		return MakeFailure(
			ERecipeManagerError::NotAuthority,
			FText::FromString(TEXT("BuildPlanAndQueueAcrossStationsFromActiveRoundRecipes must run on authority.")),
			nullptr,
			EmptyValidation,
			0);
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeManagerError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	const TArray<URecipeDataAsset*> ActiveRoundRecipes = RecipeSystem->GetActiveRoundRecipes();
	if (ActiveRoundRecipes.Num() == 0)
	{
		return MakeFailure(
			ERecipeManagerError::EmptyActiveRoundRecipes,
			FText::FromString(TEXT("No active round recipes are configured in RecipeSystem.")),
			nullptr,
			EmptyValidation,
			0);
	}

	return BuildPlanAndQueueAcrossStations(StepStations, ActiveRoundRecipes, InputItems, bClearExistingQueues);
}

FRecipeManagerResult URecipeManagerSubsystem::BuildPlanAndQueueAcrossStations(
	const TArray<AStationActorBase*>& StepStations,
	const TArray<URecipeDataAsset*>& CandidateRecipes,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueues)
{
	FRecipeValidationResult EmptyValidation;

	if (!IsAuthorityWorld())
	{
		return MakeFailure(
			ERecipeManagerError::NotAuthority,
			FText::FromString(TEXT("BuildPlanAndQueueAcrossStations must run on authority.")),
			nullptr,
			EmptyValidation,
			0);
	}

	if (StepStations.Num() == 0)
	{
		return MakeFailure(
			ERecipeManagerError::NullStepStation,
			FText::FromString(TEXT("Step station list is empty.")),
			nullptr,
			EmptyValidation,
			0);
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeManagerError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	FRecipeExecutionPlan ExecutionPlan;
	if (!RecipeSystem->TryBuildExecutionPlan(CandidateRecipes, InputItems, ExecutionPlan))
	{
		FRecipeManagerResult FailureResult = MakeFailure(
			ERecipeManagerError::NoMatchingRecipe,
			ExecutionPlan.Validation.FailureReason.IsEmpty()
				? FText::FromString(TEXT("No matching recipe for provided input items."))
				: ExecutionPlan.Validation.FailureReason,
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);

		if (ExecutionPlan.Recipe != nullptr)
		{
			FailureResult.FailureOutcome = RecipeSystem->ResolveFailureOutcome(ExecutionPlan.Recipe.Get(), ExecutionPlan.Validation);
			if (StepStations[0] != nullptr)
			{
				MaterializeFailureOutcomeOnStation(StepStations[0], FailureResult.FailureOutcome, bClearExistingQueues);
			}
		}

		return FailureResult;
	}

	return QueueExecutionPlanAcrossStations(StepStations, ExecutionPlan, bClearExistingQueues);
}

FRecipeManagerResult URecipeManagerSubsystem::QueueExecutionPlanAcrossStations(
	const TArray<AStationActorBase*>& StepStations,
	const FRecipeExecutionPlan& ExecutionPlan,
	bool bClearExistingQueues)
{
	FRecipeValidationResult EmptyValidation;

	if (ExecutionPlan.Recipe == nullptr || !ExecutionPlan.Validation.bIsValid)
	{
		return MakeFailure(
			ERecipeManagerError::InvalidExecutionPlan,
			FText::FromString(TEXT("Execution plan is invalid or unresolved.")),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	if (StepStations.Num() < ExecutionPlan.Instructions.Num())
	{
		return MakeFailure(
			ERecipeManagerError::StepStationCountMismatch,
			FText::Format(
				FText::FromString(TEXT("Not enough stations for execution plan steps ({0} stations for {1} steps).")),
				StepStations.Num(),
				ExecutionPlan.Instructions.Num()),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	TArray<FInstruction> BuiltInstructions;
	BuiltInstructions.Reserve(ExecutionPlan.Instructions.Num());
	for (int32 StepIndex = 0; StepIndex < ExecutionPlan.Instructions.Num(); ++StepIndex)
	{
		AStationActorBase* StepStation = StepStations[StepIndex];
		if (StepStation == nullptr)
		{
			return MakeFailure(
				ERecipeManagerError::NullStepStation,
				FText::Format(
					FText::FromString(TEXT("Step station at index {0} is null.")),
					StepIndex),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		if (!StepStation->HasAuthority())
		{
			return MakeFailure(
				ERecipeManagerError::StepStationNotAuthority,
				FText::Format(
					FText::FromString(TEXT("Step station at index {0} is not authority.")),
					StepIndex),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		const FRecipeInstructionData& RecipeInstruction = ExecutionPlan.Instructions[StepIndex];
		if (!RecipeInstruction.InputItem.IsValid() || !RecipeInstruction.OutputItem.IsValid())
		{
			return MakeFailure(
				ERecipeManagerError::InvalidInstruction,
				FText::Format(
					FText::FromString(TEXT("Instruction at step {0} has invalid input/output item id.")),
					StepIndex),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		const FInstruction StationInstruction = BuildStationInstruction(RecipeInstruction, ExecutionPlan.Recipe.Get());
		if (!StepStation->CanExecuteInstruction(StationInstruction))
		{
			return MakeFailure(
				ERecipeManagerError::StationCannotExecuteInstruction,
				FText::Format(
					FText::FromString(TEXT("Station at step {0} cannot execute instruction (activity mismatch).")),
					StepIndex),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		BuiltInstructions.Add(StationInstruction);
	}

	TSet<TObjectPtr<AStationActorBase>> ClearedStations;
	if (bClearExistingQueues)
	{
		for (int32 StepIndex = 0; StepIndex < BuiltInstructions.Num(); ++StepIndex)
		{
			AStationActorBase* StepStation = StepStations[StepIndex];
			if (StepStation != nullptr && !ClearedStations.Contains(StepStation))
			{
				StepStation->ClearInstructionQueue();
				ClearedStations.Add(StepStation);
			}
		}
	}

	int32 QueuedInstructions = 0;
	for (int32 StepIndex = 0; StepIndex < BuiltInstructions.Num(); ++StepIndex)
	{
		if (AStationActorBase* StepStation = StepStations[StepIndex])
		{
			StepStation->QueueInstruction(BuiltInstructions[StepIndex]);
			++QueuedInstructions;
		}
	}

	FRecipeManagerResult Result;
	Result.bSuccess = true;
	Result.ErrorCode = ERecipeManagerError::None;
	Result.Message = FText::Format(
		FText::FromString(TEXT("Queued {0} instruction(s) across {1} station step(s).")),
		QueuedInstructions,
		ExecutionPlan.Instructions.Num());
	Result.InstructionsQueued = QueuedInstructions;
	Result.ResolvedRecipe = ExecutionPlan.Recipe;
	Result.Validation = ExecutionPlan.Validation;
	return Result;
}

void URecipeManagerSubsystem::HandleFireStationProcessRequested(APlayerController* InstigatorController, AFireStation* FireStation)
{
	if (!IsAuthorityWorld() || FireStation == nullptr || !FireStation->HasAuthority())
	{
		return;
	}

	const AItemActor* HeldItemActor = FireStation->GetHeldItemActor();
	ACauldron* HeldCauldron = Cast<ACauldron>(const_cast<AItemActor*>(HeldItemActor));
	if (HeldCauldron == nullptr)
	{
		ActiveFireStationExecutions.Remove(FireStation);
		return;
	}

	if (FFireStationExecutionContext* ExistingContext = ActiveFireStationExecutions.Find(FireStation))
	{
		if (ExistingContext->Cauldron.Get() == HeldCauldron && ExistingContext->RemainingSteps > 0)
		{
			APawn* InstigatorPawn = InstigatorController ? InstigatorController->GetPawn() : nullptr;
			FireStation->TryStartProcessingHeldItem(InstigatorPawn);
			return;
		}

		ActiveFireStationExecutions.Remove(FireStation);
	}

	const UCarriableComponent* HeldCarriable = HeldItemActor ? HeldItemActor->FindComponentByClass<UCarriableComponent>() : nullptr;
	const FPrimaryAssetId HeldCauldronItemId = HeldCarriable ? HeldCarriable->GetItemId() : FPrimaryAssetId();
	if (!HeldCauldronItemId.IsValid())
	{
		return;
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return;
	}

	const TArray<URecipeDataAsset*> ActiveRoundRecipes = RecipeSystem->GetActiveRoundRecipes();
	if (ActiveRoundRecipes.Num() == 0)
	{
		return;
	}

	const TArray<FPrimaryAssetId> InputItems = HeldCauldron->GetIngredientAssetIdsSorted();
	if (InputItems.Num() == 0)
	{
		return;
	}

	FRecipeExecutionPlan ExecutionPlan;
	if (!RecipeSystem->TryBuildExecutionPlan(ActiveRoundRecipes, InputItems, ExecutionPlan))
	{
		ApplyRecipeFailureToCauldron(RecipeSystem, ExecutionPlan, HeldCauldron);
		return;
	}

	if (ExecutionPlan.Recipe == nullptr || !ExecutionPlan.Validation.bIsValid || ExecutionPlan.Instructions.Num() == 0)
	{
		return;
	}

	FireStation->ClearInstructionQueue();
	int32 QueuedSteps = 0;
	for (const FRecipeInstructionData& RecipeInstruction : ExecutionPlan.Instructions)
	{
		FInstruction StationInstruction = BuildStationInstruction(RecipeInstruction, ExecutionPlan.Recipe.Get());
		StationInstruction.InputItem = HeldCauldronItemId;
		StationInstruction.InputQuantity = 1;
		StationInstruction.OutputItem = FPrimaryAssetId();
		StationInstruction.OutputQuantity = 1;
		StationInstruction.bConsumeInputOnSuccess = false;
		StationInstruction.bProduceOutputOnSuccess = false;
		StationInstruction.bConsumeInputOnFailure = false;
		StationInstruction.FailureOutputItem = FPrimaryAssetId();
		StationInstruction.FailureOutputQuantity = 1;

		if (!FireStation->CanExecuteInstruction(StationInstruction))
		{
			FireStation->ClearInstructionQueue();
			return;
		}

		FireStation->QueueInstruction(StationInstruction);
		++QueuedSteps;
	}

	ensureMsgf(
		QueuedSteps == ExecutionPlan.Instructions.Num(),
		TEXT("Fire-station queued step count mismatch for recipe '%s' (%d queued vs %d expected)."),
		ExecutionPlan.Recipe ? *ExecutionPlan.Recipe->GetName() : TEXT("None"),
		QueuedSteps,
		ExecutionPlan.Instructions.Num());

	if (QueuedSteps <= 0)
	{
		return;
	}

	FFireStationExecutionContext Context;
	Context.Cauldron = HeldCauldron;
	Context.ExecutionPlan = ExecutionPlan;
	Context.RemainingSteps = QueuedSteps;
	ActiveFireStationExecutions.Add(FireStation, Context);

	APawn* InstigatorPawn = InstigatorController ? InstigatorController->GetPawn() : nullptr;
	FireStation->TryStartProcessingHeldItem(InstigatorPawn);
}

void URecipeManagerSubsystem::HandleFireStationInstructionProcessed(AStationActorBase* Station, const FInstruction& Instruction, bool bSuccess)
{
	(void)Instruction;

	AFireStation* FireStation = Cast<AFireStation>(Station);
	if (!IsAuthorityWorld() || FireStation == nullptr)
	{
		return;
	}

	FFireStationExecutionContext* Context = ActiveFireStationExecutions.Find(FireStation);
	if (Context == nullptr)
	{
		return;
	}

	if (!bSuccess)
	{
		const FFireStationExecutionContext FailedContext = *Context;
		ActiveFireStationExecutions.Remove(FireStation);

		FireStation->ClearInstructionQueue();

		ACauldron* Cauldron = FailedContext.Cauldron.Get();
		URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
		if (Cauldron == nullptr || !Cauldron->HasAuthority() || RecipeSystem == nullptr)
		{
			return;
		}

		const int32 TotalStepCount = FailedContext.ExecutionPlan.Instructions.Num();
		FRecipeExecutionPlan FailurePlan = FailedContext.ExecutionPlan;
		FailurePlan.Validation.bIsValid = false;
		FailurePlan.Validation.ErrorCode = ERecipeValidationError::StepNoMatch;
		FailurePlan.Validation.MatchedStepCount = FMath::Clamp(TotalStepCount - FailedContext.RemainingSteps, 0, TotalStepCount);
		FailurePlan.Validation.FirstFailedStepIndex = TotalStepCount > 0
			? FMath::Clamp(FailurePlan.Validation.MatchedStepCount, 0, TotalStepCount - 1)
			: INDEX_NONE;
		FailurePlan.Validation.FailureReason = FText::FromString(TEXT("Station instruction failed during fire-station execution."));

		ApplyRecipeFailureToCauldron(RecipeSystem, FailurePlan, Cauldron);
		return;
	}

	if (Context->RemainingSteps > 0)
	{
		--Context->RemainingSteps;
	}
	else
	{
		ensureMsgf(false, TEXT("Received fire-station instruction callback with no remaining steps on '%s'."), *FireStation->GetName());
		return;
	}

	if (Context->RemainingSteps > 0)
	{
		return;
	}

	const FFireStationExecutionContext CompletedContext = *Context;
	ActiveFireStationExecutions.Remove(FireStation);
	FireStation->ClearInstructionQueue();

	ACauldron* Cauldron = CompletedContext.Cauldron.Get();
	if (Cauldron == nullptr || !Cauldron->HasAuthority())
	{
		return;
	}

	if (!TryApplyExecutionPlanToCauldron(CompletedContext.ExecutionPlan, Cauldron))
	{
		UE_LOGFMT(MS_RecipeManagerSubsystem, Warning, "Failed applying completed fire-station execution plan to cauldron '{0}'.", Cauldron->GetName());
	}
}

void URecipeManagerSubsystem::RefreshFireStations()
{
	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	for (auto It = BoundFireStations.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	for (TActorIterator<AFireStation> It(GetWorld()); It; ++It)
	{
		RegisterFireStation(*It);
	}
}

void URecipeManagerSubsystem::HandleActorSpawned(AActor* SpawnedActor)
{
	if (!IsAuthorityWorld() || SpawnedActor == nullptr)
	{
		return;
	}

	if (AFireStation* FireStation = Cast<AFireStation>(SpawnedActor))
	{
		RegisterFireStation(FireStation);
	}
}

void URecipeManagerSubsystem::HandleFireStationDestroyed(AActor* DestroyedActor)
{
	if (!IsAuthorityWorld() || DestroyedActor == nullptr)
	{
		return;
	}

	if (AFireStation* FireStation = Cast<AFireStation>(DestroyedActor))
	{
		UnregisterFireStation(FireStation);
	}
}

void URecipeManagerSubsystem::RegisterFireStation(AFireStation* FireStation)
{
	if (!IsAuthorityWorld() || FireStation == nullptr)
	{
		return;
	}

	if (BoundFireStations.Contains(FireStation))
	{
		return;
	}

	FireStation->OnProcessRequested.AddUniqueDynamic(this, &URecipeManagerSubsystem::HandleFireStationProcessRequested);
	FireStation->OnDestroyed.AddUniqueDynamic(this, &URecipeManagerSubsystem::HandleFireStationDestroyed);
	FireStation->OnInstructionProcessed.AddUObject(this, &URecipeManagerSubsystem::HandleFireStationInstructionProcessed);
	BoundFireStations.Add(FireStation);
}

void URecipeManagerSubsystem::UnregisterFireStation(AFireStation* FireStation)
{
	if (FireStation == nullptr)
	{
		return;
	}

	FireStation->OnProcessRequested.RemoveDynamic(this, &URecipeManagerSubsystem::HandleFireStationProcessRequested);
	FireStation->OnDestroyed.RemoveDynamic(this, &URecipeManagerSubsystem::HandleFireStationDestroyed);
	FireStation->OnInstructionProcessed.RemoveAll(this);
	ActiveFireStationExecutions.Remove(FireStation);
	BoundFireStations.Remove(FireStation);
}

bool URecipeManagerSubsystem::ApplyRecipeFailureToCauldron(URecipeSystem* RecipeSystem, const FRecipeExecutionPlan& FailedPlan, ACauldron* Cauldron)
{
	if (RecipeSystem == nullptr || Cauldron == nullptr || !Cauldron->HasAuthority() || FailedPlan.Recipe == nullptr)
	{
		return false;
	}

	const URecipeDataAsset* Recipe = FailedPlan.Recipe.Get();
	const FRecipeValidationResult& Validation = FailedPlan.Validation;
	const FRecipeFailureOutcome FailureOutcome = RecipeSystem->ResolveFailureOutcome(Recipe, Validation);

	const int32 CompletedStepCount = FMath::Clamp(Validation.MatchedStepCount, 0, Recipe->Steps.Num());
	const FRecipeItemFlow CompletedFlow = RecipeSystem->BuildItemFlowForCompletedSteps(Recipe, CompletedStepCount);

	if (FailureOutcome.bConsumeMatchedInputs && CompletedFlow.ConsumedItems.Num() > 0
		&& !Cauldron->ConsumeIngredientAssetIds(CompletedFlow.ConsumedItems, true))
	{
		UE_LOGFMT(MS_RecipeManagerSubsystem, Warning, "Failed consuming matched items on recipe failure for cauldron '{0}'.", Cauldron->GetName());
		return false;
	}

	if (!FailureOutcome.bProducesFailureOutput || !FailureOutcome.FailureOutputItem.IsValid())
	{
		return true;
	}

	const int32 FailureOutputQuantity = FMath::Max(1, FailureOutcome.FailureOutputQuantity);
	if (Cauldron->GetIngredientCount() + FailureOutputQuantity > Cauldron->GetMaxIngredientCount())
	{
		UE_LOGFMT(MS_RecipeManagerSubsystem, Warning, "Cannot add failure output to cauldron '{0}': capacity exceeded.", Cauldron->GetName());
		return false;
	}

	for (int32 Index = 0; Index < FailureOutputQuantity; ++Index)
	{
		if (!Cauldron->AddContentAssetId(FailureOutcome.FailureOutputItem))
		{
			UE_LOGFMT(MS_RecipeManagerSubsystem, Warning, "Failed adding failure output item to cauldron '{0}'.", Cauldron->GetName());
			return false;
		}
	}

	return true;
}

bool URecipeManagerSubsystem::TryApplyExecutionPlanToCauldron(const FRecipeExecutionPlan& ExecutionPlan, ACauldron* Cauldron)
{
	if (Cauldron == nullptr || !Cauldron->HasAuthority() || ExecutionPlan.Recipe == nullptr || !ExecutionPlan.Validation.bIsValid)
	{
		return false;
	}

	const TArray<FPrimaryAssetId>& ConsumedItems = ExecutionPlan.ItemFlow.ConsumedItems;
	const TArray<FPrimaryAssetId>& ProducedItems = ExecutionPlan.ItemFlow.NetProducedItems.Num() > 0
		? ExecutionPlan.ItemFlow.NetProducedItems
		: ExecutionPlan.ItemFlow.GeneratedItems;

	for (const FPrimaryAssetId& ProducedItem : ProducedItems)
	{
		if (!ProducedItem.IsValid())
		{
			ensureMsgf(false, TEXT("Execution plan contains invalid produced item id for recipe '%s'."), *ExecutionPlan.Recipe->GetName());
			return false;
		}
	}

	const int32 ProjectedCount = FMath::Max(0, Cauldron->GetIngredientCount() - ConsumedItems.Num()) + ProducedItems.Num();
	if (ProjectedCount > Cauldron->GetMaxIngredientCount())
	{
		ensureMsgf(false, TEXT("Cauldron capacity exceeded while applying recipe '%s' (%d > %d)."), *ExecutionPlan.Recipe->GetName(), ProjectedCount, Cauldron->GetMaxIngredientCount());
		return false;
	}

	if (ConsumedItems.Num() > 0 && !Cauldron->ConsumeIngredientAssetIds(ConsumedItems, true))
	{
		return false;
	}

	for (const FPrimaryAssetId& ProducedItem : ProducedItems)
	{
		if (!Cauldron->AddContentAssetId(ProducedItem))
		{
			return false;
		}
	}

	return true;
}

bool URecipeManagerSubsystem::MaterializeFailureOutcomeOnStation(
	AStationActorBase* Station,
	const FRecipeFailureOutcome& FailureOutcome,
	bool bClearExistingQueue)
{
	if (Station == nullptr || !Station->HasAuthority())
	{
		return false;
	}

	FPrimaryAssetId EffectiveFailureOutput = FailureOutcome.FailureOutputItem;
	int32 EffectiveFailureQuantity = FMath::Max(1, FailureOutcome.FailureOutputQuantity);
	bool bProducesFailureOutput = FailureOutcome.bProducesFailureOutput;

	if (Station->HasFailureOutputOverride())
	{
		EffectiveFailureOutput = Station->GetFailureOutputOverrideItem();
		EffectiveFailureQuantity = Station->GetFailureOutputOverrideQuantity();
		bProducesFailureOutput = EffectiveFailureOutput.IsValid();
	}

	if (!FailureOutcome.bConsumeMatchedInputs && !bProducesFailureOutput)
	{
		return true;
	}

	return Station->ApplyFailureOutcome(
		bProducesFailureOutput ? EffectiveFailureOutput : FPrimaryAssetId(),
		bProducesFailureOutput ? EffectiveFailureQuantity : 1,
		FailureOutcome.bConsumeMatchedInputs,
		bClearExistingQueue);
}

FRecipeManagerResult URecipeManagerSubsystem::MakeFailure(
	ERecipeManagerError ErrorCode,
	const FText& Message,
	const URecipeDataAsset* Recipe,
	const FRecipeValidationResult& Validation,
	int32 InstructionsQueued) const
{
	FRecipeManagerResult Result;
	Result.bSuccess = false;
	Result.ErrorCode = ErrorCode;
	Result.Message = Message;
	Result.InstructionsQueued = InstructionsQueued;
	Result.ResolvedRecipe = const_cast<URecipeDataAsset*>(Recipe);
	Result.Validation = Validation;
	return Result;
}

bool URecipeManagerSubsystem::IsAuthorityWorld() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_Standalone || NetMode == NM_ListenServer || NetMode == NM_DedicatedServer;
}


