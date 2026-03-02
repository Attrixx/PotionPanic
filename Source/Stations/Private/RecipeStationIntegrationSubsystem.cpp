// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeStationIntegrationSubsystem.h"
#include "Instruction.h"
#include "ItemAsset.h"
#include "RecipeSystem.h"
#include "StationActorBase.h"

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
			// TODO (Nath): When failure materialization is implemented, resolve output with priority:
			// 1) Station-specific failure override (if any), 2) Recipe FailureOutputItem, 3) global default failure item ("Amalgame").
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

	if (bClearExistingQueue)
	{
		Station->ClearInstructionQueue();
	}

	int32 QueuedInstructions = 0;
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
				QueuedInstructions);
		}

		FInstruction StationInstruction;
		StationInstruction.InputItem = RecipeInstruction.InputItem;
		StationInstruction.InputQuantity = FMath::Max(1, RecipeInstruction.InputQuantity);
		StationInstruction.OutputItem = RecipeInstruction.OutputItem;
		StationInstruction.OutputQuantity = FMath::Max(1, RecipeInstruction.OutputQuantity);
		StationInstruction.Activity = RecipeInstruction.Activity;
		StationInstruction.ProcessingDuration = RecipeInstruction.ProcessingDuration;
		StationInstruction.bRequiresProximity = RecipeInstruction.bRequiresProximity;
		if (ExecutionPlan.Recipe != nullptr)
		{
			StationInstruction.bConsumeInputOnFailure = ExecutionPlan.Recipe->bConsumeMatchedInputsOnFailure;
			if (ExecutionPlan.Recipe->FailureOutputItem != nullptr)
			{
				StationInstruction.FailureOutputItem = ExecutionPlan.Recipe->FailureOutputItem->GetPrimaryAssetId();
				StationInstruction.FailureOutputQuantity = FMath::Max(1, ExecutionPlan.Recipe->FailureOutputQuantity);
			}
		}

		if (!Station->CanExecuteInstruction(StationInstruction))
		{
			return MakeFailure(
				ERecipeStationIntegrationError::StationCannotExecuteInstruction,
				FText::Format(
					FText::FromString(TEXT("Station cannot execute instruction at step {0} (activity mismatch).")),
					StepIndex),
				ExecutionPlan.Recipe.Get(),
				ExecutionPlan.Validation,
				QueuedInstructions);
		}

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
