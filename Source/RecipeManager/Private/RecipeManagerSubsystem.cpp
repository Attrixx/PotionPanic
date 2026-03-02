// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeManagerSubsystem.h"
#include "Instruction.h"
#include "Cauldron.h"
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

	for (const TWeakObjectPtr<AStationActorBase>& WeakStation : BoundFireStations)
	{
		if (AStationActorBase* FireStation = WeakStation.Get())
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
enum class EStationInstructionValidationFailure : uint8
{
	None,
	InvalidInputOutputIds,
	ActivityMismatch,
	InputFilterMismatch
};

struct FStationInstructionValidationResult
{
	EStationInstructionValidationFailure Failure = EStationInstructionValidationFailure::None;
	FInstruction StationInstruction;
};

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

FStationInstructionValidationResult ValidateStationInstructionForStep(
	const FRecipeInstructionData& RecipeInstruction,
	const URecipeDataAsset* Recipe,
	const AStationActorBase* Station)
{
	FStationInstructionValidationResult ValidationResult;

	if (!RecipeInstruction.InputItem.IsValid() || !RecipeInstruction.OutputItem.IsValid())
	{
		ValidationResult.Failure = EStationInstructionValidationFailure::InvalidInputOutputIds;
		return ValidationResult;
	}

	ValidationResult.StationInstruction = BuildStationInstruction(RecipeInstruction, Recipe);
	if (Station != nullptr && !Station->CanExecuteInstruction(ValidationResult.StationInstruction))
	{
		ValidationResult.Failure = EStationInstructionValidationFailure::ActivityMismatch;
		return ValidationResult;
	}

	if (Station != nullptr && !Station->CanPlaceItem(ValidationResult.StationInstruction.InputItem))
	{
		ValidationResult.Failure = EStationInstructionValidationFailure::InputFilterMismatch;
		return ValidationResult;
	}

	return ValidationResult;
}

ERecipeManagerError MapInstructionValidationFailureToManagerError(EStationInstructionValidationFailure Failure)
{
	switch (Failure)
	{
	case EStationInstructionValidationFailure::InvalidInputOutputIds:
		return ERecipeManagerError::InvalidInstruction;
	case EStationInstructionValidationFailure::ActivityMismatch:
	case EStationInstructionValidationFailure::InputFilterMismatch:
		return ERecipeManagerError::StationCannotExecuteInstruction;
	case EStationInstructionValidationFailure::None:
	default:
		return ERecipeManagerError::None;
	}
}

FText BuildInstructionValidationFailureMessage(EStationInstructionValidationFailure Failure, int32 StepIndex, bool bAcrossStations)
{
	switch (Failure)
	{
	case EStationInstructionValidationFailure::InvalidInputOutputIds:
		return FText::Format(
			FText::FromString(TEXT("Instruction at step {0} has invalid input/output item id.")),
			StepIndex);
	case EStationInstructionValidationFailure::ActivityMismatch:
		return FText::Format(
			FText::FromString(
				bAcrossStations
					? TEXT("Station at step {0} cannot execute instruction (activity mismatch).")
					: TEXT("Station cannot execute instruction at step {0} (activity mismatch).")),
			StepIndex);
	case EStationInstructionValidationFailure::InputFilterMismatch:
		return FText::Format(
			FText::FromString(
				bAcrossStations
					? TEXT("Station at step {0} cannot accept instruction input item (item filter mismatch).")
					: TEXT("Station cannot accept input item for instruction at step {0} (item filter mismatch).")),
			StepIndex);
	case EStationInstructionValidationFailure::None:
	default:
		return FText::GetEmpty();
	}
}

FText ResolveNoMatchingRecipeMessage(const FRecipeExecutionPlan& ExecutionPlan)
{
	return ExecutionPlan.Validation.FailureReason.IsEmpty()
		? FText::FromString(TEXT("No matching recipe for provided input items."))
		: ExecutionPlan.Validation.FailureReason;
}

bool RestoreCauldronContents(ACauldron* Cauldron, const TArray<FPrimaryAssetId>& SnapshotContents)
{
	if (Cauldron == nullptr || !Cauldron->HasAuthority())
	{
		return false;
	}

	Cauldron->ClearIngredients();
	for (const FPrimaryAssetId& ContentId : SnapshotContents)
	{
		if (!Cauldron->AddContentAssetId(ContentId))
		{
			return false;
		}
	}

	return true;
}

bool TryRollbackCauldronContents(ACauldron* Cauldron, const TArray<FPrimaryAssetId>& SnapshotContents, const TCHAR* ContextLabel)
{
	if (RestoreCauldronContents(Cauldron, SnapshotContents))
	{
		return true;
	}

	UE_LOGFMT(
		MS_RecipeManagerSubsystem,
		Error,
		"Failed rolling back cauldron '{0}' after {1}.",
		Cauldron ? Cauldron->GetName() : TEXT("Unknown"),
		ContextLabel ? FString(ContextLabel) : FString(TEXT("unknown error")));
	return false;
}

FPrimaryAssetId ResolveHeldItemId(const AItemActor* HeldItemActor)
{
	return HeldItemActor ? HeldItemActor->GetItemAssetId() : FPrimaryAssetId();
}

void ConfigureFireStationInstructionForCauldron(FInstruction& StationInstruction, const FPrimaryAssetId& HeldCauldronItemId)
{
	StationInstruction.InputItem = HeldCauldronItemId;
	StationInstruction.InputQuantity = 1;
	StationInstruction.OutputItem = FPrimaryAssetId();
	StationInstruction.OutputQuantity = 1;
	StationInstruction.bConsumeInputOnSuccess = false;
	StationInstruction.bProduceOutputOnSuccess = false;
	StationInstruction.bConsumeInputOnFailure = false;
	StationInstruction.FailureOutputItem = FPrimaryAssetId();
	StationInstruction.FailureOutputQuantity = 1;
}

bool TryQueueFireStationExecutionPlan(
	AStationActorBase* FireStation,
	const FRecipeExecutionPlan& ExecutionPlan,
	const FPrimaryAssetId& HeldCauldronItemId,
	int32& OutQueuedSteps)
{
	OutQueuedSteps = 0;
	if (FireStation == nullptr)
	{
		return false;
	}

	FireStation->ClearInstructionQueue();
	for (const FRecipeInstructionData& RecipeInstruction : ExecutionPlan.Instructions)
	{
		FInstruction StationInstruction = BuildStationInstruction(RecipeInstruction, ExecutionPlan.Recipe.Get());
		ConfigureFireStationInstructionForCauldron(StationInstruction, HeldCauldronItemId);

		if (!FireStation->CanExecuteInstruction(StationInstruction))
		{
			FireStation->ClearInstructionQueue();
			return false;
		}

		FireStation->QueueInstruction(StationInstruction);
		++OutQueuedSteps;
	}

	ensureMsgf(
		OutQueuedSteps == ExecutionPlan.Instructions.Num(),
		TEXT("Fire-station queued step count mismatch for recipe '%s' (%d queued vs %d expected)."),
		ExecutionPlan.Recipe ? *ExecutionPlan.Recipe->GetName() : TEXT("None"),
		OutQueuedSteps,
		ExecutionPlan.Instructions.Num());

	if (OutQueuedSteps <= 0)
	{
		FireStation->ClearInstructionQueue();
		return false;
	}

	return true;
}

FRecipeExecutionPlan BuildFireStationFailurePlan(const FRecipeExecutionPlan& SourcePlan, int32 RemainingSteps)
{
	const int32 TotalStepCount = SourcePlan.Instructions.Num();
	FRecipeExecutionPlan FailurePlan = SourcePlan;
	FailurePlan.Validation.bIsValid = false;
	FailurePlan.Validation.ErrorCode = ERecipeValidationError::StepNoMatch;
	FailurePlan.Validation.MatchedStepCount = FMath::Clamp(TotalStepCount - RemainingSteps, 0, TotalStepCount);
	FailurePlan.Validation.FirstFailedStepIndex = TotalStepCount > 0
		? FMath::Clamp(FailurePlan.Validation.MatchedStepCount, 0, TotalStepCount - 1)
		: INDEX_NONE;
	FailurePlan.Validation.FailureReason = FText::FromString(TEXT("Station instruction failed during fire-station execution."));
	return FailurePlan;
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

	URecipeSystem* RecipeSystem = GetRecipeSystem();
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
			ResolveNoMatchingRecipeMessage(ExecutionPlan),
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

	URecipeSystem* RecipeSystem = GetRecipeSystem();
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
		const FStationInstructionValidationResult ValidationResult = ValidateStationInstructionForStep(
			RecipeInstruction,
			ExecutionPlan.Recipe.Get(),
			Station);
		if (ValidationResult.Failure != EStationInstructionValidationFailure::None)
		{
			return MakeFailure(
				MapInstructionValidationFailureToManagerError(ValidationResult.Failure),
				BuildInstructionValidationFailureMessage(ValidationResult.Failure, StepIndex, false),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		StationInstructions.Add(ValidationResult.StationInstruction);
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

	URecipeSystem* RecipeSystem = GetRecipeSystem();
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

	URecipeSystem* RecipeSystem = GetRecipeSystem();
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
			ResolveNoMatchingRecipeMessage(ExecutionPlan),
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

		const FStationInstructionValidationResult ValidationResult = ValidateStationInstructionForStep(
			ExecutionPlan.Instructions[StepIndex],
			ExecutionPlan.Recipe.Get(),
			StepStation);
		if (ValidationResult.Failure != EStationInstructionValidationFailure::None)
		{
			return MakeFailure(
				MapInstructionValidationFailureToManagerError(ValidationResult.Failure),
				BuildInstructionValidationFailureMessage(ValidationResult.Failure, StepIndex, true),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				0);
		}

		BuiltInstructions.Add(ValidationResult.StationInstruction);
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

void URecipeManagerSubsystem::StartFireStationProcessing(AStationActorBase* FireStation, APlayerController* InstigatorController) const
{
	if (FireStation == nullptr)
	{
		return;
	}

	APawn* InstigatorPawn = InstigatorController ? InstigatorController->GetPawn() : nullptr;
	FireStation->TryStartProcessingHeldItem(InstigatorPawn);
}

bool URecipeManagerSubsystem::TryResumeExistingFireStationExecution(
	AStationActorBase* FireStation,
	ACauldron* HeldCauldron,
	APlayerController* InstigatorController)
{
	if (FireStation == nullptr)
	{
		return false;
	}

	FFireStationExecutionContext* ExistingContext = ActiveFireStationExecutions.Find(FireStation);
	if (ExistingContext == nullptr)
	{
		return false;
	}

	if (HeldCauldron != nullptr && ExistingContext->Cauldron.Get() == HeldCauldron && ExistingContext->RemainingSteps > 0)
	{
		StartFireStationProcessing(FireStation, InstigatorController);
		return true;
	}

	ActiveFireStationExecutions.Remove(FireStation);
	return false;
}

bool URecipeManagerSubsystem::TryBuildFireStationExecutionPlan(ACauldron* HeldCauldron, FRecipeExecutionPlan& OutExecutionPlan)
{
	OutExecutionPlan = FRecipeExecutionPlan();
	if (HeldCauldron == nullptr)
	{
		return false;
	}

	URecipeSystem* RecipeSystem = GetRecipeSystem();
	if (RecipeSystem == nullptr)
	{
		return false;
	}

	const TArray<URecipeDataAsset*> ActiveRoundRecipes = RecipeSystem->GetActiveRoundRecipes();
	if (ActiveRoundRecipes.Num() == 0)
	{
		return false;
	}

	const TArray<FPrimaryAssetId> InputItems = HeldCauldron->GetIngredientAssetIdsSorted();
	if (InputItems.Num() == 0)
	{
		return false;
	}

	if (!RecipeSystem->TryBuildExecutionPlan(ActiveRoundRecipes, InputItems, OutExecutionPlan))
	{
		ApplyRecipeFailureToCauldron(RecipeSystem, OutExecutionPlan, HeldCauldron);
		return false;
	}

	return OutExecutionPlan.Recipe != nullptr
		&& OutExecutionPlan.Validation.bIsValid
		&& OutExecutionPlan.Instructions.Num() > 0;
}

bool URecipeManagerSubsystem::TryStartNewFireStationExecution(
	AStationActorBase* FireStation,
	ACauldron* HeldCauldron,
	const FPrimaryAssetId& HeldCauldronItemId,
	const FRecipeExecutionPlan& ExecutionPlan)
{
	if (FireStation == nullptr || HeldCauldron == nullptr)
	{
		return false;
	}

	int32 QueuedSteps = 0;
	if (!TryQueueFireStationExecutionPlan(FireStation, ExecutionPlan, HeldCauldronItemId, QueuedSteps))
	{
		return false;
	}

	FFireStationExecutionContext Context;
	Context.Cauldron = HeldCauldron;
	Context.ExecutionPlan = ExecutionPlan;
	Context.RemainingSteps = QueuedSteps;
	ActiveFireStationExecutions.Add(FireStation, Context);
	return true;
}

bool URecipeManagerSubsystem::ConsumeProcessedFireStationStep(
	AStationActorBase* FireStation,
	FFireStationExecutionContext& Context)
{
	if (Context.RemainingSteps <= 0)
	{
		ensureMsgf(
			false,
			TEXT("Received fire-station instruction callback with no remaining steps on '%s'."),
			FireStation ? *FireStation->GetName() : TEXT("Unknown"));
		return false;
	}

	--Context.RemainingSteps;
	return Context.RemainingSteps == 0;
}

void URecipeManagerSubsystem::HandleFailedFireStationExecution(
	AStationActorBase* FireStation,
	const FFireStationExecutionContext& FailedContext)
{
	if (FireStation == nullptr)
	{
		return;
	}

	FireStation->ClearInstructionQueue();

	ACauldron* Cauldron = FailedContext.Cauldron.Get();
	URecipeSystem* RecipeSystem = GetRecipeSystem();
	if (Cauldron == nullptr || !Cauldron->HasAuthority() || RecipeSystem == nullptr)
	{
		return;
	}

	const FRecipeExecutionPlan FailurePlan = BuildFireStationFailurePlan(FailedContext.ExecutionPlan, FailedContext.RemainingSteps);
	ApplyRecipeFailureToCauldron(RecipeSystem, FailurePlan, Cauldron);
}

void URecipeManagerSubsystem::HandleCompletedFireStationExecution(
	AStationActorBase* FireStation,
	const FFireStationExecutionContext& CompletedContext)
{
	if (FireStation == nullptr)
	{
		return;
	}

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

void URecipeManagerSubsystem::HandleFireStationProcessRequested(APlayerController* InstigatorController, AStationActorBase* FireStation)
{
	if (!IsAuthorityWorld()
		|| FireStation == nullptr
		|| !FireStation->HasAuthority()
		|| FireStation->GetStationKind() != EStationKind::Fire)
	{
		return;
	}

	AItemActor* HeldItemActor = FireStation->GetHeldItemActor();
	ACauldron* HeldCauldron = Cast<ACauldron>(HeldItemActor);
	if (HeldCauldron == nullptr)
	{
		ActiveFireStationExecutions.Remove(FireStation);
		return;
	}

	if (TryResumeExistingFireStationExecution(FireStation, HeldCauldron, InstigatorController))
	{
		return;
	}

	const FPrimaryAssetId HeldCauldronItemId = ResolveHeldItemId(HeldItemActor);
	if (!HeldCauldronItemId.IsValid())
	{
		return;
	}

	FRecipeExecutionPlan ExecutionPlan;
	if (!TryBuildFireStationExecutionPlan(HeldCauldron, ExecutionPlan))
	{
		return;
	}

	if (!TryStartNewFireStationExecution(FireStation, HeldCauldron, HeldCauldronItemId, ExecutionPlan))
	{
		return;
	}

	StartFireStationProcessing(FireStation, InstigatorController);
}

void URecipeManagerSubsystem::HandleFireStationInstructionProcessed(AStationActorBase* Station, const FInstruction& Instruction, bool bSuccess)
{
	(void)Instruction;

	if (!IsAuthorityWorld() || Station == nullptr || Station->GetStationKind() != EStationKind::Fire)
	{
		return;
	}

	FFireStationExecutionContext* Context = ActiveFireStationExecutions.Find(Station);
	if (Context == nullptr)
	{
		return;
	}

	if (!bSuccess)
	{
		const FFireStationExecutionContext FailedContext = *Context;
		ActiveFireStationExecutions.Remove(Station);
		HandleFailedFireStationExecution(Station, FailedContext);
		return;
	}

	if (!ConsumeProcessedFireStationStep(Station, *Context))
	{
		return;
	}

	const FFireStationExecutionContext CompletedContext = *Context;
	ActiveFireStationExecutions.Remove(Station);
	HandleCompletedFireStationExecution(Station, CompletedContext);
}

void URecipeManagerSubsystem::RefreshFireStations()
{
	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	for (auto It = BoundFireStations.CreateIterator(); It; ++It)
	{
		if (!It->IsValid() || It->Get()->GetStationKind() != EStationKind::Fire)
		{
			It.RemoveCurrent();
		}
	}

	for (TActorIterator<AStationActorBase> It(GetWorld()); It; ++It)
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

	if (AStationActorBase* FireStation = Cast<AStationActorBase>(SpawnedActor))
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

	if (AStationActorBase* FireStation = Cast<AStationActorBase>(DestroyedActor))
	{
		UnregisterFireStation(FireStation);
	}
}

void URecipeManagerSubsystem::RegisterFireStation(AStationActorBase* FireStation)
{
	if (!IsAuthorityWorld()
		|| FireStation == nullptr
		|| FireStation->GetStationKind() != EStationKind::Fire)
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

void URecipeManagerSubsystem::UnregisterFireStation(AStationActorBase* FireStation)
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

	const TArray<FPrimaryAssetId> OriginalContents = Cauldron->GetIngredientAssetIds();

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
		TryRollbackCauldronContents(Cauldron, OriginalContents, TEXT("failure-output capacity validation"));
		return false;
	}

	for (int32 Index = 0; Index < FailureOutputQuantity; ++Index)
	{
		if (!Cauldron->AddContentAssetId(FailureOutcome.FailureOutputItem))
		{
			UE_LOGFMT(MS_RecipeManagerSubsystem, Warning, "Failed adding failure output item to cauldron '{0}'.", Cauldron->GetName());
			TryRollbackCauldronContents(Cauldron, OriginalContents, TEXT("failure-output add error"));
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

	const TArray<FPrimaryAssetId> OriginalContents = Cauldron->GetIngredientAssetIds();

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
			TryRollbackCauldronContents(Cauldron, OriginalContents, TEXT("execution-plan output add error"));
			return false;
		}
	}

	return true;
}

URecipeSystem* URecipeManagerSubsystem::GetRecipeSystem() const
{
	if (UWorld* World = GetWorld())
	{
		return World->GetSubsystem<URecipeSystem>();
	}

	return nullptr;
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


