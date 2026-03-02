// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RecipeSystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "RecipeStationIntegrationSubsystem.generated.h"

class AStationActorBase;
class URecipeDataAsset;

UENUM(BlueprintType)
enum class ERecipeStationIntegrationError : uint8
{
	None UMETA(DisplayName = "None"),
	NullStation UMETA(DisplayName = "NullStation"),
	NotAuthority UMETA(DisplayName = "NotAuthority"),
	RecipeSystemUnavailable UMETA(DisplayName = "RecipeSystemUnavailable"),
	EmptyActiveRoundRecipes UMETA(DisplayName = "EmptyActiveRoundRecipes"),
	NoMatchingRecipe UMETA(DisplayName = "NoMatchingRecipe"),
	InvalidExecutionPlan UMETA(DisplayName = "InvalidExecutionPlan"),
	InvalidInstruction UMETA(DisplayName = "InvalidInstruction"),
	NullStepStation UMETA(DisplayName = "NullStepStation"),
	StepStationNotAuthority UMETA(DisplayName = "StepStationNotAuthority"),
	StepStationCountMismatch UMETA(DisplayName = "StepStationCountMismatch"),
	StationCannotExecuteInstruction UMETA(DisplayName = "StationCannotExecuteInstruction")
};

USTRUCT(BlueprintType)
struct STATIONS_API FRecipeStationIntegrationResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	ERecipeStationIntegrationError ErrorCode = ERecipeStationIntegrationError::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	int32 InstructionsQueued = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	TObjectPtr<URecipeDataAsset> ResolvedRecipe = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	FRecipeValidationResult Validation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	FRecipeFailureOutcome FailureOutcome;
};

/**
 * Server-side adapter between RecipeSystem execution plans and station instruction queues.
 * Keeps Recipes decoupled from station runtime execution details.
 */
UCLASS()
class STATIONS_API URecipeStationIntegrationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Uses recipes currently active in RecipeSystem runtime rotation. */
	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeStationIntegrationResult BuildPlanAndQueueFromActiveRoundRecipes(
		AStationActorBase* Station,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueue = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeStationIntegrationResult BuildPlanAndQueueInstructions(
		AStationActorBase* Station,
		const TArray<URecipeDataAsset*>& CandidateRecipes,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueue = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeStationIntegrationResult QueueExecutionPlan(
		AStationActorBase* Station,
		const FRecipeExecutionPlan& ExecutionPlan,
		bool bClearExistingQueue = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeStationIntegrationResult BuildPlanAndQueueAcrossStationsFromActiveRoundRecipes(
		const TArray<AStationActorBase*>& StepStations,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueues = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeStationIntegrationResult BuildPlanAndQueueAcrossStations(
		const TArray<AStationActorBase*>& StepStations,
		const TArray<URecipeDataAsset*>& CandidateRecipes,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueues = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeStationIntegrationResult QueueExecutionPlanAcrossStations(
		const TArray<AStationActorBase*>& StepStations,
		const FRecipeExecutionPlan& ExecutionPlan,
		bool bClearExistingQueues = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	bool MaterializeFailureOutcomeOnStation(
		AStationActorBase* Station,
		const FRecipeFailureOutcome& FailureOutcome,
		bool bClearExistingQueue = true);

private:
	FRecipeStationIntegrationResult MakeFailure(
		ERecipeStationIntegrationError ErrorCode,
		const FText& Message,
		const URecipeDataAsset* Recipe,
		const FRecipeValidationResult& Validation,
		int32 InstructionsQueued) const;
};
