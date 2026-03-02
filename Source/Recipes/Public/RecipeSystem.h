// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractionBase.h"
#include "RecipeAsset.h"
#include "Subsystems/WorldSubsystem.h"
#include <random>
#include "RecipeSystem.generated.h"

class UItemTransformation;
class UActivityAsset;

UENUM(BlueprintType)
enum class ERecipeValidationError : uint8
{
	None UMETA(DisplayName = "None"),
	NullRecipe UMETA(DisplayName = "NullRecipe"),
	InvalidRecipeDefinition UMETA(DisplayName = "InvalidRecipeDefinition"),
	StrictInputCountMismatch UMETA(DisplayName = "StrictInputCountMismatch"),
	EmptyInput UMETA(DisplayName = "EmptyInput"),
	StepNull UMETA(DisplayName = "StepNull"),
	StepNoMatch UMETA(DisplayName = "StepNoMatch"),
	InvalidInputItemId UMETA(DisplayName = "InvalidInputItemId"),
	InputAssetResolveFailed UMETA(DisplayName = "InputAssetResolveFailed"),
	InputItemMismatch UMETA(DisplayName = "InputItemMismatch"),
	MissingItemDataTags UMETA(DisplayName = "MissingItemDataTags"),
	MissingTransformationFlags UMETA(DisplayName = "MissingTransformationFlags"),
	IngredientExpected UMETA(DisplayName = "IngredientExpected"),
	IngredientNotProcessed UMETA(DisplayName = "IngredientNotProcessed"),
	MissingIngredientStateFlags UMETA(DisplayName = "MissingIngredientStateFlags"),
	MissingIngredientStateTags UMETA(DisplayName = "MissingIngredientStateTags"),
	Unknown UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeInstructionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	FPrimaryAssetId InputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction", meta = (ClampMin = "1"))
	int32 InputQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	FPrimaryAssetId OutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction", meta = (ClampMin = "1"))
	int32 OutputQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	TObjectPtr<UActivityAsset> Activity = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	TObjectPtr<UInteractionDefinitionAsset> InteractionDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction", meta = (ClampMin = "0.0"))
	float ProcessingDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	bool bRequiresProximity = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	int32 BaseStepScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	float InteractionScoreMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Instruction")
	int32 FailurePenalty = 0;
};

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeValidationResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Validation")
	ERecipeValidationError ErrorCode = ERecipeValidationError::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Validation")
	bool bIsValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Validation")
	int32 MatchedStepCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Validation")
	int32 FirstFailedStepIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Validation")
	FText FailureReason;
};

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeItemFlow
{
	GENERATED_BODY()

	/** Raw consumed inputs across all steps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Flow")
	TArray<FPrimaryAssetId> ConsumedItems;

	/** Raw generated outputs across all steps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Flow")
	TArray<FPrimaryAssetId> GeneratedItems;

	/** Net inputs removed after cancelling intermediate transforms. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Flow")
	TArray<FPrimaryAssetId> NetRemovedItems;

	/** Net outputs produced after cancelling intermediate transforms. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Flow")
	TArray<FPrimaryAssetId> NetProducedItems;
};

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeExecutionPlan
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Plan")
	TObjectPtr<URecipeDataAsset> Recipe = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Plan")
	FRecipeValidationResult Validation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Plan")
	TArray<FRecipeInstructionData> Instructions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Plan")
	FRecipeItemFlow ItemFlow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Plan")
	int32 BaseRecipeScore = 0;
};

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeConcurrentPlanResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Concurrency")
	bool bHasAnyPlan = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Concurrency")
	int32 PlannedRecipeCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Concurrency")
	TArray<FRecipeExecutionPlan> Plans;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Concurrency")
	TArray<FPrimaryAssetId> ConsumedItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Concurrency")
	TArray<FPrimaryAssetId> RemainingItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Concurrency")
	FText Message;
};

UENUM(BlueprintType)
enum class ERecipeConcurrentSolveMode : uint8
{
	Greedy UMETA(DisplayName = "Greedy"),
	OptimalBranchAndBound UMETA(DisplayName = "OptimalBranchAndBound")
};

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeFailureOutcome
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Failure")
	bool bConsumeMatchedInputs = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Failure")
	int32 MatchedInputCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Failure")
	bool bProducesFailureOutput = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Failure")
	FPrimaryAssetId FailureOutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Failure", meta = (ClampMin = "1"))
	int32 FailureOutputQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Failure")
	FText Message;
};

USTRUCT(BlueprintType)
struct RECIPES_API FRecipeScoreContext
{
	GENERATED_BODY()

	/** 0..1 ratio of remaining time for this recipe execution. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Scoring", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TimeRemainingRatio = 1.0f;

	/** Max bonus applied from remaining time ratio. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Scoring", meta = (ClampMin = "0"))
	int32 TimeBonusMax = 0;

	/** Global recipe score multiplier (difficulty, tuning). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Scoring", meta = (ClampMin = "0.0"))
	float DifficultyMultiplier = 1.0f;

	/** Team/player streak multiplier. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe|Scoring", meta = (ClampMin = "0.0"))
	float StreakMultiplier = 1.0f;
};

/**
 * Pure recipe domain subsystem.
 * Owns recipe validation/planning/scoring and never executes station runtime.
 */
UCLASS()
class RECIPES_API URecipeSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Registers the full recipe catalog available for the current world/session. */
	UFUNCTION(BlueprintCallable, Category = "Recipes|Runtime")
	void SetRecipeCatalog(const TArray<URecipeDataAsset*>& InRecipeCatalog);

	/** Starts a new level/round recipe rotation from the catalog and stores active runtime recipes. */
	UFUNCTION(BlueprintCallable, Category = "Recipes|Runtime")
	bool StartRoundRecipeRotation(int32 LevelIndex, int32 RoundIndex, int32 MaxRecipes = 0);

	UFUNCTION(BlueprintPure, Category = "Recipes|Runtime")
	TArray<URecipeDataAsset*> GetRecipeCatalog() const;

	UFUNCTION(BlueprintPure, Category = "Recipes|Runtime")
	TArray<URecipeDataAsset*> GetActiveRoundRecipes() const;

	UFUNCTION(BlueprintPure, Category = "Recipes|Runtime")
	bool HasActiveRoundRecipes() const { return ActiveRoundRecipes.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Recipes|Runtime")
	int32 GetActiveRoundSeed() const { return ActiveRoundSeed; }

	UFUNCTION(BlueprintCallable, Category = "Recipes")
	TArray<URecipeDataAsset*> GetShuffledRecipes(const TArray<URecipeDataAsset*>& InRecipes) const;

	/** Deterministic shuffle helper for server-driven multiplayer rotations and tests. */
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	TArray<URecipeDataAsset*> GetShuffledRecipesWithSeed(const TArray<URecipeDataAsset*>& InRecipes, int32 Seed) const;

	/** Overrides internal random state used by GetShuffledRecipes(). */
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	void ReseedShuffleGenerator(int32 NewSeed) const;

	/** Generates a unique seed for a level/round context (different across runs). */
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	int32 GenerateUniqueShuffleSeed(int32 LevelIndex, int32 RoundIndex) const;

	/** Convenience helper: generates a unique seed and returns the corresponding shuffle. */
	UFUNCTION(BlueprintCallable, Category = "Recipes")
	TArray<URecipeDataAsset*> GetShuffledRecipesForLevelRound(const TArray<URecipeDataAsset*>& InRecipes, int32 LevelIndex, int32 RoundIndex, int32& OutSeed) const;

	UFUNCTION(BlueprintPure, Category = "Recipes")
	int32 GetLastGeneratedShuffleSeed() const { return LastGeneratedShuffleSeed; }

	UFUNCTION(BlueprintCallable, Category = "Recipes")
	FRecipeValidationResult ValidateRecipeInputs(const URecipeDataAsset* Recipe, const TArray<FPrimaryAssetId>& InputItems) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes")
	TArray<FRecipeInstructionData> BuildInstructions(const URecipeDataAsset* Recipe) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes")
	FRecipeItemFlow BuildItemFlow(const URecipeDataAsset* Recipe) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes")
	FRecipeItemFlow BuildItemFlowForCompletedSteps(const URecipeDataAsset* Recipe, int32 CompletedStepCount) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes|Failure")
	FRecipeFailureOutcome ResolveFailureOutcome(const URecipeDataAsset* Recipe, const FRecipeValidationResult& Validation) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes")
	bool TryResolveBestRecipe(const TArray<URecipeDataAsset*>& CandidateRecipes, const TArray<FPrimaryAssetId>& InputItems, URecipeDataAsset*& OutRecipe, FRecipeValidationResult& OutValidation) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes")
	bool TryBuildExecutionPlan(const TArray<URecipeDataAsset*>& CandidateRecipes, const TArray<FPrimaryAssetId>& InputItems, FRecipeExecutionPlan& OutPlan) const;

	/** Builds as many recipe plans as possible from the same input pool (Overcooked-style parallel prep). */
	UFUNCTION(BlueprintCallable, Category = "Recipes|Concurrency")
	FRecipeConcurrentPlanResult BuildConcurrentExecutionPlans(
		const TArray<URecipeDataAsset*>& CandidateRecipes,
		const TArray<FPrimaryAssetId>& AvailableInputItems,
		int32 MaxConcurrentRecipes = 0,
		ERecipeConcurrentSolveMode SolveMode = ERecipeConcurrentSolveMode::Greedy) const;

	/** Convenience helper using active round recipes as candidates. */
	UFUNCTION(BlueprintCallable, Category = "Recipes|Concurrency")
	FRecipeConcurrentPlanResult BuildConcurrentExecutionPlansFromActiveRoundRecipes(
		const TArray<FPrimaryAssetId>& AvailableInputItems,
		int32 MaxConcurrentRecipes = 0,
		ERecipeConcurrentSolveMode SolveMode = ERecipeConcurrentSolveMode::Greedy) const;

	/** Uses designer-configured defaults for solve mode and max concurrent recipes. */
	UFUNCTION(BlueprintCallable, Category = "Recipes|Concurrency")
	FRecipeConcurrentPlanResult BuildConcurrentExecutionPlansWithDefaults(const TArray<FPrimaryAssetId>& AvailableInputItems) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes|Scoring")
	int32 ComputeFinalScore(const URecipeDataAsset* Recipe, const TArray<FInteractionOutput>& InteractionOutputs, bool bRecipeCompleted) const;

	UFUNCTION(BlueprintCallable, Category = "Recipes|Scoring")
	int32 ComputeFinalScoreWithContext(const URecipeDataAsset* Recipe, const TArray<FInteractionOutput>& InteractionOutputs, bool bRecipeCompleted, const FRecipeScoreContext& ScoreContext) const;

private:
	void EnsureShuffleGeneratorInitialized() const;
	bool DoesItemMatchStep(const FPrimaryAssetId& ItemId, const UItemTransformation* Step, FText& OutFailureReason, ERecipeValidationError* OutErrorCode = nullptr) const;
	int32 ComputeInteractionContribution(const URecipeDataAsset* Recipe, const TArray<FInteractionOutput>& InteractionOutputs) const;
	bool TryBuildConsumableInputList(const URecipeDataAsset* Recipe, const TMap<FPrimaryAssetId, int32>& AvailablePool, TArray<FPrimaryAssetId>& OutSelectedInputs) const;
	static void ConsumeInputListFromPool(const TArray<FPrimaryAssetId>& InputList, TMap<FPrimaryAssetId, int32>& InOutPool);
	static void ExpandPoolToItemArray(const TMap<FPrimaryAssetId, int32>& Pool, TArray<FPrimaryAssetId>& OutItems);

	mutable std::mt19937 ShuffleGenerator;
	mutable bool bShuffleGeneratorInitialized = false;
	mutable int32 ShuffleInvocationCounter = 0;
	mutable int32 LastGeneratedShuffleSeed = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<URecipeDataAsset>> RecipeCatalog;

	UPROPERTY(Transient)
	TArray<TObjectPtr<URecipeDataAsset>> ActiveRoundRecipes;

	UPROPERTY(Transient)
	int32 ActiveRoundSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipes|Concurrency", meta = (AllowPrivateAccess = "true"))
	ERecipeConcurrentSolveMode DefaultConcurrentSolveMode = ERecipeConcurrentSolveMode::Greedy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipes|Concurrency", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 DefaultMaxConcurrentRecipes = 0;
};
