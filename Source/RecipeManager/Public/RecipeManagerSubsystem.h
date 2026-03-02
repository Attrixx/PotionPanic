// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RecipeSystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "RecipeManagerSubsystem.generated.h"

class AFireStation;
class AStationActorBase;
class ACauldron;
class APlayerController;
class AActor;
class URecipeDataAsset;
struct FInstruction;

UENUM(BlueprintType)
enum class ERecipeManagerError : uint8
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
struct RECIPEMANAGER_API FRecipeManagerResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Integration")
	ERecipeManagerError ErrorCode = ERecipeManagerError::None;

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
 * Contract:
 * - Single runtime orchestrator between RecipeSystem and stations/orders.
 * - Recipes define/validate/score only (what to do), never where/how to execute.
 * - Stations execute queued instructions only, and never resolve recipes.
 */
UCLASS()
class RECIPEMANAGER_API URecipeManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Uses recipes currently active in RecipeSystem runtime rotation. */
	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeManagerResult BuildPlanAndQueueFromActiveRoundRecipes(
		AStationActorBase* Station,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueue = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeManagerResult BuildPlanAndQueueInstructions(
		AStationActorBase* Station,
		const TArray<URecipeDataAsset*>& CandidateRecipes,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueue = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeManagerResult QueueExecutionPlan(
		AStationActorBase* Station,
		const FRecipeExecutionPlan& ExecutionPlan,
		bool bClearExistingQueue = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeManagerResult BuildPlanAndQueueAcrossStationsFromActiveRoundRecipes(
		const TArray<AStationActorBase*>& StepStations,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueues = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeManagerResult BuildPlanAndQueueAcrossStations(
		const TArray<AStationActorBase*>& StepStations,
		const TArray<URecipeDataAsset*>& CandidateRecipes,
		const TArray<FPrimaryAssetId>& InputItems,
		bool bClearExistingQueues = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	FRecipeManagerResult QueueExecutionPlanAcrossStations(
		const TArray<AStationActorBase*>& StepStations,
		const FRecipeExecutionPlan& ExecutionPlan,
		bool bClearExistingQueues = true);

	UFUNCTION(BlueprintCallable, Category = "Recipe|Integration")
	bool MaterializeFailureOutcomeOnStation(
		AStationActorBase* Station,
		const FRecipeFailureOutcome& FailureOutcome,
		bool bClearExistingQueue = true);

private:
	UFUNCTION()
	void HandleFireStationProcessRequested(APlayerController* InstigatorController, AFireStation* FireStation);
	UFUNCTION()
	void HandleFireStationDestroyed(AActor* DestroyedActor);
	void HandleFireStationInstructionProcessed(AStationActorBase* Station, const FInstruction& Instruction, bool bSuccess);

	void RefreshFireStations();
	void HandleActorSpawned(AActor* SpawnedActor);
	void RegisterFireStation(AFireStation* FireStation);
	void UnregisterFireStation(AFireStation* FireStation);
	bool ApplyRecipeFailureToCauldron(URecipeSystem* RecipeSystem, const FRecipeExecutionPlan& FailedPlan, ACauldron* Cauldron);
	bool TryApplyExecutionPlanToCauldron(const FRecipeExecutionPlan& ExecutionPlan, ACauldron* Cauldron);
	bool IsAuthorityWorld() const;

	FRecipeManagerResult MakeFailure(
		ERecipeManagerError ErrorCode,
		const FText& Message,
		const URecipeDataAsset* Recipe,
		const FRecipeValidationResult& Validation,
		int32 InstructionsQueued) const;

private:
	struct FFireStationExecutionContext
	{
		TWeakObjectPtr<ACauldron> Cauldron;
		FRecipeExecutionPlan ExecutionPlan;
		int32 RemainingSteps = 0;
	};

	TSet<TWeakObjectPtr<AFireStation>> BoundFireStations;
	TMap<TWeakObjectPtr<AFireStation>, FFireStationExecutionContext> ActiveFireStationExecutions;
	FDelegateHandle ActorSpawnedHandle;
};


