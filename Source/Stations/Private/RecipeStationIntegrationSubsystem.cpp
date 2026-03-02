// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeStationIntegrationSubsystem.h"
#include "Instruction.h"
#include "ItemAsset.h"
#include "RecipeSystem.h"
#include "StationActorBase.h"

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

FRecipeStationIntegrationResult URecipeStationIntegrationSubsystem::BuildPlanAndQueueInstructions(
	AStationActorBase* Station,
	const TArray<URecipeDataAsset*>& CandidateRecipes,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueue)
{
	FRecipeValidationResult EmptyValidation;

	if (Station == nullptr)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::NullStation,
			FText::FromString(TEXT("Station is null.")),
			nullptr,
			EmptyValidation,
			0);
	}

	if (!Station->HasAuthority())
	{
		return MakeFailure(
			ERecipeStationIntegrationError::NotAuthority,
			FText::FromString(TEXT("BuildPlanAndQueueInstructions must run on authority.")),
			nullptr,
			EmptyValidation,
			0);
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	FRecipeExecutionPlan ExecutionPlan;
	if (!RecipeSystem->TryBuildExecutionPlan(CandidateRecipes, InputItems, ExecutionPlan))
	{
		FRecipeStationIntegrationResult FailureResult = MakeFailure(
			ERecipeStationIntegrationError::NoMatchingRecipe,
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

FRecipeStationIntegrationResult URecipeStationIntegrationSubsystem::BuildPlanAndQueueFromActiveRoundRecipes(
	AStationActorBase* Station,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueue)
{
	FRecipeValidationResult EmptyValidation;

	if (Station == nullptr)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::NullStation,
			FText::FromString(TEXT("Station is null.")),
			nullptr,
			EmptyValidation,
			0);
	}

	if (!Station->HasAuthority())
	{
		return MakeFailure(
			ERecipeStationIntegrationError::NotAuthority,
			FText::FromString(TEXT("BuildPlanAndQueueFromActiveRoundRecipes must run on authority.")),
			nullptr,
			EmptyValidation,
			0);
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	const TArray<URecipeDataAsset*> ActiveRoundRecipes = RecipeSystem->GetActiveRoundRecipes();
	if (ActiveRoundRecipes.Num() == 0)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::EmptyActiveRoundRecipes,
			FText::FromString(TEXT("No active round recipes are configured in RecipeSystem.")),
			nullptr,
			EmptyValidation,
			0);
	}

	return BuildPlanAndQueueInstructions(Station, ActiveRoundRecipes, InputItems, bClearExistingQueue);
}

FRecipeStationIntegrationResult URecipeStationIntegrationSubsystem::QueueExecutionPlan(
	AStationActorBase* Station,
	const FRecipeExecutionPlan& ExecutionPlan,
	bool bClearExistingQueue)
{
	FRecipeValidationResult EmptyValidation;

	if (Station == nullptr)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::NullStation,
			FText::FromString(TEXT("Station is null.")),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	if (!Station->HasAuthority())
	{
		return MakeFailure(
			ERecipeStationIntegrationError::NotAuthority,
			FText::FromString(TEXT("QueueExecutionPlan must run on authority.")),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	if (ExecutionPlan.Recipe == nullptr || !ExecutionPlan.Validation.bIsValid)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::InvalidExecutionPlan,
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
				ERecipeStationIntegrationError::InvalidInstruction,
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
				ERecipeStationIntegrationError::StationCannotExecuteInstruction,
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

	FRecipeStationIntegrationResult Result;
	Result.bSuccess = true;
	Result.ErrorCode = ERecipeStationIntegrationError::None;
	Result.Message = FText::Format(
		FText::FromString(TEXT("Queued {0} instruction(s) on station '{1}'.")),
		QueuedInstructions,
		FText::FromString(Station->GetName()));
	Result.InstructionsQueued = QueuedInstructions;
	Result.ResolvedRecipe = ExecutionPlan.Recipe;
	Result.Validation = ExecutionPlan.Validation;
	return Result;
}

FRecipeStationIntegrationResult URecipeStationIntegrationSubsystem::BuildPlanAndQueueAcrossStationsFromActiveRoundRecipes(
	const TArray<AStationActorBase*>& StepStations,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueues)
{
	FRecipeValidationResult EmptyValidation;

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	const TArray<URecipeDataAsset*> ActiveRoundRecipes = RecipeSystem->GetActiveRoundRecipes();
	if (ActiveRoundRecipes.Num() == 0)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::EmptyActiveRoundRecipes,
			FText::FromString(TEXT("No active round recipes are configured in RecipeSystem.")),
			nullptr,
			EmptyValidation,
			0);
	}

	return BuildPlanAndQueueAcrossStations(StepStations, ActiveRoundRecipes, InputItems, bClearExistingQueues);
}

FRecipeStationIntegrationResult URecipeStationIntegrationSubsystem::BuildPlanAndQueueAcrossStations(
	const TArray<AStationActorBase*>& StepStations,
	const TArray<URecipeDataAsset*>& CandidateRecipes,
	const TArray<FPrimaryAssetId>& InputItems,
	bool bClearExistingQueues)
{
	FRecipeValidationResult EmptyValidation;

	if (StepStations.Num() == 0)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::NullStepStation,
			FText::FromString(TEXT("Step station list is empty.")),
			nullptr,
			EmptyValidation,
			0);
	}

	URecipeSystem* RecipeSystem = GetWorld() ? GetWorld()->GetSubsystem<URecipeSystem>() : nullptr;
	if (RecipeSystem == nullptr)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::RecipeSystemUnavailable,
			FText::FromString(TEXT("RecipeSystem subsystem is unavailable.")),
			nullptr,
			EmptyValidation,
			0);
	}

	FRecipeExecutionPlan ExecutionPlan;
	if (!RecipeSystem->TryBuildExecutionPlan(CandidateRecipes, InputItems, ExecutionPlan))
	{
		FRecipeStationIntegrationResult FailureResult = MakeFailure(
			ERecipeStationIntegrationError::NoMatchingRecipe,
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

FRecipeStationIntegrationResult URecipeStationIntegrationSubsystem::QueueExecutionPlanAcrossStations(
	const TArray<AStationActorBase*>& StepStations,
	const FRecipeExecutionPlan& ExecutionPlan,
	bool bClearExistingQueues)
{
	FRecipeValidationResult EmptyValidation;

	if (ExecutionPlan.Recipe == nullptr || !ExecutionPlan.Validation.bIsValid)
	{
		return MakeFailure(
			ERecipeStationIntegrationError::InvalidExecutionPlan,
			FText::FromString(TEXT("Execution plan is invalid or unresolved.")),
			ExecutionPlan.Recipe.Get(),
			ExecutionPlan.Validation,
			0);
	}

	if (StepStations.Num() < ExecutionPlan.Instructions.Num())
	{
		return MakeFailure(
			ERecipeStationIntegrationError::StepStationCountMismatch,
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
				ERecipeStationIntegrationError::NullStepStation,
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
				ERecipeStationIntegrationError::StepStationNotAuthority,
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
				ERecipeStationIntegrationError::InvalidInstruction,
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
				ERecipeStationIntegrationError::StationCannotExecuteInstruction,
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

	FRecipeStationIntegrationResult Result;
	Result.bSuccess = true;
	Result.ErrorCode = ERecipeStationIntegrationError::None;
	Result.Message = FText::Format(
		FText::FromString(TEXT("Queued {0} instruction(s) across {1} station step(s).")),
		QueuedInstructions,
		ExecutionPlan.Instructions.Num());
	Result.InstructionsQueued = QueuedInstructions;
	Result.ResolvedRecipe = ExecutionPlan.Recipe;
	Result.Validation = ExecutionPlan.Validation;
	return Result;
}

bool URecipeStationIntegrationSubsystem::MaterializeFailureOutcomeOnStation(
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

FRecipeStationIntegrationResult URecipeStationIntegrationSubsystem::MakeFailure(
	ERecipeStationIntegrationError ErrorCode,
	const FText& Message,
	const URecipeDataAsset* Recipe,
	const FRecipeValidationResult& Validation,
	int32 InstructionsQueued) const
{
	FRecipeStationIntegrationResult Result;
	Result.bSuccess = false;
	Result.ErrorCode = ErrorCode;
	Result.Message = Message;
	Result.InstructionsQueued = InstructionsQueued;
	Result.ResolvedRecipe = const_cast<URecipeDataAsset*>(Recipe);
	Result.Validation = Validation;
	return Result;
}
