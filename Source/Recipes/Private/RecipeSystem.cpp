#include "RecipeSystem.h"
#include "Engine/AssetManager.h"
#include "HAL/PlatformTime.h"
#include "IngredientData.h"
#include "ItemAsset.h"
#include "ItemTransformation.h"
#include <algorithm>
#include <random>

static float GetInteractionDefinitionDuration(const UInteractionDefinitionAsset* Definition)
{
	if (Definition == nullptr)
	{
		return 0.0f;
	}

	return Definition->Type == EInteractionType::QTE
		? Definition->QTE.MaxDurationSeconds
		: Definition->IFT.MaxDurationSeconds;
}

static UItemAsset* ResolveItemAssetFromId(const FPrimaryAssetId& ItemId)
{
	if (!ItemId.IsValid())
	{
		return nullptr;
	}

	UItemAsset* ItemAsset = UAssetManager::Get().GetPrimaryAssetObject<UItemAsset>(ItemId);
	if (ItemAsset != nullptr)
	{
		return ItemAsset;
	}

	const FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ItemId);
	return Cast<UItemAsset>(AssetPath.TryLoad());
}

template <typename T>
static bool ContainsAll(const TArray<T>& Container, const TArray<T>& RequiredValues)
{
	for (const T& RequiredValue : RequiredValues)
	{
		if (!Container.Contains(RequiredValue))
		{
			return false;
		}
	}

	return true;
}

static void AddToMapCount(TMap<FPrimaryAssetId, int32>& CounterMap, const FPrimaryAssetId& ItemId, int32 Quantity = 1)
{
	if (!ItemId.IsValid())
	{
		return;
	}

	if (Quantity <= 0)
	{
		return;
	}

	const int32 CurrentCount = CounterMap.FindRef(ItemId);
	CounterMap.Add(ItemId, CurrentCount + Quantity);
}

static int32 GetSafeInputQuantity(const UItemTransformation* Step)
{
	return Step ? FMath::Max(1, Step->InputQuantity) : 1;
}

static int32 GetSafeOutputQuantity(const UItemTransformation* Step)
{
	return Step ? FMath::Max(1, Step->OutputQuantity) : 1;
}

template <typename T>
static TArray<TObjectPtr<T>> ToObjectPtrArray(const TArray<T*>& Source)
{
	TArray<TObjectPtr<T>> Result;
	Result.Reserve(Source.Num());
	for (T* Entry : Source)
	{
		if (Entry != nullptr)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

template <typename T>
static TArray<T*> ToRawPointerArray(const TArray<TObjectPtr<T>>& Source)
{
	TArray<T*> Result;
	Result.Reserve(Source.Num());
	for (T* Entry : Source)
	{
		if (Entry != nullptr)
		{
			Result.Add(Entry);
		}
	}
	return Result;
}

static FRecipeItemFlow BuildItemFlowInternal(const URecipeDataAsset* Recipe, int32 MaxStepCount)
{
	FRecipeItemFlow Flow;

	if (Recipe == nullptr || MaxStepCount <= 0)
	{
		return Flow;
	}

	const int32 StepCount = FMath::Min(MaxStepCount, Recipe->Steps.Num());
	TMap<FPrimaryAssetId, int32> ConsumedCountMap;
	TMap<FPrimaryAssetId, int32> GeneratedCountMap;

	for (int32 StepIndex = 0; StepIndex < StepCount; ++StepIndex)
	{
		const UItemTransformation* Step = Recipe->Steps[StepIndex];
		if (Step == nullptr)
		{
			continue;
		}

		if (Step->InputItem)
		{
			const FPrimaryAssetId ConsumedId = Step->InputItem->GetPrimaryAssetId();
			const int32 InputQuantity = GetSafeInputQuantity(Step);
			for (int32 QuantityIndex = 0; QuantityIndex < InputQuantity; ++QuantityIndex)
			{
				Flow.ConsumedItems.Add(ConsumedId);
			}
			AddToMapCount(ConsumedCountMap, ConsumedId, InputQuantity);
		}

		if (Step->OutputItem)
		{
			const FPrimaryAssetId GeneratedId = Step->OutputItem->GetPrimaryAssetId();
			const int32 OutputQuantity = GetSafeOutputQuantity(Step);
			for (int32 QuantityIndex = 0; QuantityIndex < OutputQuantity; ++QuantityIndex)
			{
				Flow.GeneratedItems.Add(GeneratedId);
			}
			AddToMapCount(GeneratedCountMap, GeneratedId, OutputQuantity);
		}
	}

	TSet<FPrimaryAssetId> AllIds;
	for (const TPair<FPrimaryAssetId, int32>& Pair : ConsumedCountMap)
	{
		AllIds.Add(Pair.Key);
	}
	for (const TPair<FPrimaryAssetId, int32>& Pair : GeneratedCountMap)
	{
		AllIds.Add(Pair.Key);
	}

	for (const FPrimaryAssetId& ItemId : AllIds)
	{
		const int32 NetDelta = GeneratedCountMap.FindRef(ItemId) - ConsumedCountMap.FindRef(ItemId);
		if (NetDelta > 0)
		{
			for (int32 Index = 0; Index < NetDelta; ++Index)
			{
				Flow.NetProducedItems.Add(ItemId);
			}
		}
		else if (NetDelta < 0)
		{
			for (int32 Index = 0; Index < -NetDelta; ++Index)
			{
				Flow.NetRemovedItems.Add(ItemId);
			}
		}
	}

	return Flow;
}

void URecipeSystem::SetRecipeCatalog(const TArray<URecipeDataAsset*>& InRecipeCatalog)
{
	RecipeCatalog = ToObjectPtrArray(InRecipeCatalog);
}

bool URecipeSystem::StartRoundRecipeRotation(int32 LevelIndex, int32 RoundIndex, int32 MaxRecipes)
{
	ActiveRoundRecipes.Reset();
	ActiveRoundSeed = 0;

	if (RecipeCatalog.Num() == 0)
	{
		return false;
	}

	const TArray<URecipeDataAsset*> CatalogRaw = ToRawPointerArray(RecipeCatalog);
	int32 GeneratedSeed = 0;
	TArray<URecipeDataAsset*> ShuffledRecipes = GetShuffledRecipesForLevelRound(CatalogRaw, LevelIndex, RoundIndex, GeneratedSeed);
	if (ShuffledRecipes.Num() == 0)
	{
		return false;
	}

	if (MaxRecipes > 0 && ShuffledRecipes.Num() > MaxRecipes)
	{
		ShuffledRecipes.SetNum(MaxRecipes, EAllowShrinking::No);
	}

	ActiveRoundRecipes = ToObjectPtrArray(ShuffledRecipes);
	ActiveRoundSeed = GeneratedSeed;
	return ActiveRoundRecipes.Num() > 0;
}

TArray<URecipeDataAsset*> URecipeSystem::GetRecipeCatalog() const
{
	return ToRawPointerArray(RecipeCatalog);
}

TArray<URecipeDataAsset*> URecipeSystem::GetActiveRoundRecipes() const
{
	return ToRawPointerArray(ActiveRoundRecipes);
}

TArray<URecipeDataAsset*> URecipeSystem::GetShuffledRecipes(const TArray<URecipeDataAsset*>& InRecipes) const
{
	TArray<URecipeDataAsset*> ShuffledRecipes = InRecipes;

	if (ShuffledRecipes.Num() <= 1)
	{
		return ShuffledRecipes;
	}

	EnsureShuffleGeneratorInitialized();
	std::shuffle(ShuffledRecipes.GetData(), ShuffledRecipes.GetData() + ShuffledRecipes.Num(), ShuffleGenerator);

	return ShuffledRecipes;
}

TArray<URecipeDataAsset*> URecipeSystem::GetShuffledRecipesWithSeed(const TArray<URecipeDataAsset*>& InRecipes, int32 Seed) const
{
	TArray<URecipeDataAsset*> ShuffledRecipes = InRecipes;

	if (ShuffledRecipes.Num() <= 1)
	{
		return ShuffledRecipes;
	}

	std::mt19937 SeededGenerator(static_cast<std::mt19937::result_type>(Seed));
	std::shuffle(ShuffledRecipes.GetData(), ShuffledRecipes.GetData() + ShuffledRecipes.Num(), SeededGenerator);

	return ShuffledRecipes;
}

void URecipeSystem::ReseedShuffleGenerator(int32 NewSeed) const
{
	ShuffleGenerator.seed(static_cast<std::mt19937::result_type>(NewSeed));
	bShuffleGeneratorInitialized = true;
	LastGeneratedShuffleSeed = NewSeed;
}

void URecipeSystem::EnsureShuffleGeneratorInitialized() const
{
	if (bShuffleGeneratorInitialized)
	{
		return;
	}

	std::random_device RandomDevice;
	ShuffleGenerator.seed(RandomDevice());
	bShuffleGeneratorInitialized = true;
}

int32 URecipeSystem::GenerateUniqueShuffleSeed(int32 LevelIndex, int32 RoundIndex) const
{
	const uint64 UtcTicks = static_cast<uint64>(FDateTime::UtcNow().GetTicks());
	const uint64 CycleTicks = FPlatformTime::Cycles64();
	const uint32 InvocationHash = ::GetTypeHash(++ShuffleInvocationCounter);

	uint32 CombinedHash = ::GetTypeHash(UtcTicks);
	CombinedHash = HashCombineFast(CombinedHash, ::GetTypeHash(CycleTicks));
	CombinedHash = HashCombineFast(CombinedHash, InvocationHash);
	CombinedHash = HashCombineFast(CombinedHash, ::GetTypeHash(LevelIndex));
	CombinedHash = HashCombineFast(CombinedHash, ::GetTypeHash(RoundIndex));
	CombinedHash = HashCombineFast(CombinedHash, ::GetTypeHash(reinterpret_cast<UPTRINT>(this)));

	int32 GeneratedSeed = static_cast<int32>(CombinedHash & 0x7fffffff);
	if (GeneratedSeed == 0)
	{
		GeneratedSeed = 1;
	}

	LastGeneratedShuffleSeed = GeneratedSeed;
	return GeneratedSeed;
}

TArray<URecipeDataAsset*> URecipeSystem::GetShuffledRecipesForLevelRound(const TArray<URecipeDataAsset*>& InRecipes, int32 LevelIndex, int32 RoundIndex, int32& OutSeed) const
{
	OutSeed = GenerateUniqueShuffleSeed(LevelIndex, RoundIndex);
	return GetShuffledRecipesWithSeed(InRecipes, OutSeed);
}

FRecipeValidationResult URecipeSystem::ValidateRecipeInputs(const URecipeDataAsset* Recipe, const TArray<FPrimaryAssetId>& InputItems) const
{
	FRecipeValidationResult Validation;
	Validation.ErrorCode = ERecipeValidationError::None;
	Validation.FirstFailedStepIndex = INDEX_NONE;

	auto SetFailure = [&Validation](ERecipeValidationError ErrorCode, int32 StepIndex, const FText& Reason)
	{
		Validation.ErrorCode = ErrorCode;
		Validation.FirstFailedStepIndex = StepIndex;
		Validation.FailureReason = Reason;
		Validation.bIsValid = false;
	};

	if (Recipe == nullptr)
	{
		SetFailure(ERecipeValidationError::NullRecipe, INDEX_NONE, FText::FromString(TEXT("Recipe is null.")));
		return Validation;
	}

	FText DefinitionFailureReason;
	if (!Recipe->IsRecipeDefinitionValid(DefinitionFailureReason))
	{
		SetFailure(ERecipeValidationError::InvalidRecipeDefinition, INDEX_NONE, DefinitionFailureReason);
		return Validation;
	}

	int32 RequiredInputCount = 0;
	for (const UItemTransformation* Step : Recipe->Steps)
	{
		RequiredInputCount += GetSafeInputQuantity(Step);
	}

	const int32 StepCount = Recipe->Steps.Num();
	if (Recipe->bStrictLinearOrder && InputItems.Num() != RequiredInputCount)
	{
		SetFailure(ERecipeValidationError::StrictInputCountMismatch, INDEX_NONE, FText::Format(
			FText::FromString(TEXT("Strict recipe expects {0} input(s), got {1}.")),
			RequiredInputCount,
			InputItems.Num()));
		return Validation;
	}

	if (!Recipe->bStrictLinearOrder && InputItems.Num() == 0)
	{
		SetFailure(ERecipeValidationError::EmptyInput, INDEX_NONE, FText::FromString(TEXT("Recipe input list is empty.")));
		return Validation;
	}

	TArray<int32> UsedIndices;
	UsedIndices.Reserve(InputItems.Num());

	int32 StrictCursor = 0;

	for (int32 StepIndex = 0; StepIndex < StepCount; ++StepIndex)
	{
		const UItemTransformation* Step = Recipe->Steps[StepIndex];
		if (Step == nullptr)
		{
			SetFailure(ERecipeValidationError::StepNull, StepIndex, FText::Format(
				FText::FromString(TEXT("Step {0} is null.")),
				StepIndex));
			return Validation;
		}

		const int32 StepRequiredCount = GetSafeInputQuantity(Step);

		if (Recipe->bStrictLinearOrder)
		{
			for (int32 QuantityIndex = 0; QuantityIndex < StepRequiredCount; ++QuantityIndex)
			{
				if (!InputItems.IsValidIndex(StrictCursor))
				{
					SetFailure(ERecipeValidationError::StrictInputCountMismatch, StepIndex, FText::FromString(TEXT("Strict recipe input cursor exceeded provided inputs.")));
					return Validation;
				}

				FText FailureReason;
				ERecipeValidationError ErrorCode = ERecipeValidationError::StepNoMatch;
				if (!DoesItemMatchStep(InputItems[StrictCursor], Step, FailureReason, &ErrorCode))
				{
					if (FailureReason.IsEmpty())
					{
						FailureReason = FText::Format(
							FText::FromString(TEXT("Input at strict position {0} does not match step {1}.")),
							StrictCursor,
							StepIndex);
					}

					SetFailure(ErrorCode, StepIndex, FailureReason);
					return Validation;
				}

				++StrictCursor;
				++Validation.MatchedStepCount;
			}
			continue;
		}

		int32 StepMatchedCount = 0;
		for (int32 ItemIndex = 0; ItemIndex < InputItems.Num(); ++ItemIndex)
		{
			if (UsedIndices.Contains(ItemIndex))
			{
				continue;
			}

			FText FailureReason;
			if (!DoesItemMatchStep(InputItems[ItemIndex], Step, FailureReason))
			{
				continue;
			}

			UsedIndices.Add(ItemIndex);
			++StepMatchedCount;
			++Validation.MatchedStepCount;

			if (StepMatchedCount >= StepRequiredCount)
			{
				break;
			}
		}

		if (StepMatchedCount < StepRequiredCount)
		{
			SetFailure(
				ERecipeValidationError::StepNoMatch,
				StepIndex,
				FText::Format(
					FText::FromString(TEXT("Step {0} requires {1} matching input(s), found {2}.")),
					StepIndex,
					StepRequiredCount,
					StepMatchedCount));
			return Validation;
		}
	}

	Validation.ErrorCode = ERecipeValidationError::None;
	Validation.bIsValid = true;
	return Validation;
}

TArray<FRecipeInstructionData> URecipeSystem::BuildInstructions(const URecipeDataAsset* Recipe) const
{
	TArray<FRecipeInstructionData> Instructions;
	if (Recipe == nullptr)
	{
		return Instructions;
	}

	Instructions.Reserve(Recipe->Steps.Num());
	for (const UItemTransformation* Step : Recipe->Steps)
	{
		if (Step == nullptr)
		{
			continue;
		}

		FRecipeInstructionData Instruction;
		if (Step->InputItem)
		{
			Instruction.InputItem = Step->InputItem->GetPrimaryAssetId();
		}
		Instruction.InputQuantity = GetSafeInputQuantity(Step);

		if (Step->OutputItem)
		{
			Instruction.OutputItem = Step->OutputItem->GetPrimaryAssetId();
		}
		Instruction.OutputQuantity = GetSafeOutputQuantity(Step);

		Instruction.Activity = Step->Activity;
		Instruction.InteractionDefinition = Step->InteractionDefinition;
		Instruction.ProcessingDuration = Step->ProcessingDuration;
		if (Instruction.ProcessingDuration <= 0.0f)
		{
			Instruction.ProcessingDuration = GetInteractionDefinitionDuration(Instruction.InteractionDefinition);
		}
		Instruction.bRequiresProximity = Step->bRequiresProximity;
		Instruction.BaseStepScore = Step->BaseStepScore;
		Instruction.InteractionScoreMultiplier = Step->InteractionScoreMultiplier;
		Instruction.FailurePenalty = Step->FailurePenalty;
		Instructions.Add(Instruction);
	}

	return Instructions;
}

FRecipeItemFlow URecipeSystem::BuildItemFlow(const URecipeDataAsset* Recipe) const
{
	return BuildItemFlowInternal(Recipe, Recipe ? Recipe->Steps.Num() : 0);
}

FRecipeItemFlow URecipeSystem::BuildItemFlowForCompletedSteps(const URecipeDataAsset* Recipe, int32 CompletedStepCount) const
{
	return BuildItemFlowInternal(Recipe, FMath::Max(0, CompletedStepCount));
}

FRecipeFailureOutcome URecipeSystem::ResolveFailureOutcome(const URecipeDataAsset* Recipe, const FRecipeValidationResult& Validation) const
{
	FRecipeFailureOutcome Outcome;

	if (Recipe == nullptr)
	{
		Outcome.Message = FText::FromString(TEXT("Recipe is null. No failure outcome resolved."));
		return Outcome;
	}

	if (Validation.bIsValid)
	{
		Outcome.Message = FText::FromString(TEXT("Validation is successful. Failure outcome is not applicable."));
		return Outcome;
	}

	Outcome.MatchedInputCount = FMath::Max(0, Validation.MatchedStepCount);
	Outcome.bConsumeMatchedInputs = Recipe->bConsumeMatchedInputsOnFailure && Outcome.MatchedInputCount > 0;

	// TODO (Nath): For now ResolveFailureOutcome returns the recipe-level failure output only (global "Amalgame" strategy).
	// TODO (Nath): Add contextual failure output resolution hook (last station used) at integration/runtime level.
	if (Recipe->FailureOutputItem != nullptr && Outcome.MatchedInputCount > 0)
	{
		Outcome.bProducesFailureOutput = true;
		Outcome.FailureOutputItem = Recipe->FailureOutputItem->GetPrimaryAssetId();
		Outcome.FailureOutputQuantity = FMath::Max(1, Recipe->FailureOutputQuantity);
	}

	Outcome.Message = Validation.FailureReason.IsEmpty()
		? FText::FromString(TEXT("Recipe failed."))
		: Validation.FailureReason;

	return Outcome;
}

bool URecipeSystem::TryResolveBestRecipe(const TArray<URecipeDataAsset*>& CandidateRecipes, const TArray<FPrimaryAssetId>& InputItems, URecipeDataAsset*& OutRecipe, FRecipeValidationResult& OutValidation) const
{
	OutRecipe = nullptr;
	OutValidation = FRecipeValidationResult{};

	int32 BestMatchedSteps = INDEX_NONE;
	int32 BestRecipeStepCount = INDEX_NONE;
	FRecipeValidationResult BestPartialValidation;
	URecipeDataAsset* BestPartialRecipe = nullptr;

	for (URecipeDataAsset* CandidateRecipe : CandidateRecipes)
	{
		if (CandidateRecipe == nullptr)
		{
			continue;
		}

		const FRecipeValidationResult Validation = ValidateRecipeInputs(CandidateRecipe, InputItems);
		if (Validation.bIsValid)
		{
			const int32 CandidateMatched = Validation.MatchedStepCount;
			const int32 CandidateStepCount = CandidateRecipe->Steps.Num();
			const bool bIsBetter = OutRecipe == nullptr
				|| CandidateMatched > BestMatchedSteps
				|| (CandidateMatched == BestMatchedSteps && CandidateStepCount > BestRecipeStepCount);

			if (bIsBetter)
			{
				OutRecipe = CandidateRecipe;
				OutValidation = Validation;
				BestMatchedSteps = CandidateMatched;
				BestRecipeStepCount = CandidateStepCount;
			}
		}
		else if (OutRecipe == nullptr)
		{
			if (Validation.MatchedStepCount > BestPartialValidation.MatchedStepCount)
			{
				BestPartialValidation = Validation;
				BestPartialRecipe = CandidateRecipe;
			}
		}
	}

	if (OutRecipe != nullptr)
	{
		return true;
	}

	OutRecipe = BestPartialRecipe;
	OutValidation = BestPartialValidation;
	if (OutValidation.FailureReason.IsEmpty())
	{
		OutValidation.FailureReason = FText::FromString(TEXT("No candidate recipe matched the provided inputs."));
	}

	return false;
}

bool URecipeSystem::TryBuildExecutionPlan(const TArray<URecipeDataAsset*>& CandidateRecipes, const TArray<FPrimaryAssetId>& InputItems, FRecipeExecutionPlan& OutPlan) const
{
	OutPlan = FRecipeExecutionPlan{};

	URecipeDataAsset* ResolvedRecipe = nullptr;
	FRecipeValidationResult Validation;
	if (!TryResolveBestRecipe(CandidateRecipes, InputItems, ResolvedRecipe, Validation))
	{
		OutPlan.Recipe = ResolvedRecipe;
		OutPlan.Validation = Validation;
		return false;
	}

	OutPlan.Recipe = ResolvedRecipe;
	OutPlan.Validation = Validation;
	OutPlan.Instructions = BuildInstructions(ResolvedRecipe);
	OutPlan.ItemFlow = BuildItemFlow(ResolvedRecipe);
	OutPlan.BaseRecipeScore = ComputeFinalScore(ResolvedRecipe, TArray<FInteractionOutput>{}, true);
	return true;
}

int32 URecipeSystem::ComputeInteractionContribution(const URecipeDataAsset* Recipe, const TArray<FInteractionOutput>& InteractionOutputs) const
{
	if (Recipe == nullptr || InteractionOutputs.Num() == 0)
	{
		return 0;
	}

	TArray<int32> WeightedContributions;
	WeightedContributions.Reserve(InteractionOutputs.Num());

	for (int32 StepIndex = 0; StepIndex < Recipe->Steps.Num(); ++StepIndex)
	{
		if (!InteractionOutputs.IsValidIndex(StepIndex))
		{
			continue;
		}

		const UItemTransformation* Step = Recipe->Steps[StepIndex];
		if (Step == nullptr)
		{
			continue;
		}

		const FInteractionOutput& InteractionOutput = InteractionOutputs[StepIndex];
		const int32 WeightedContribution = FMath::RoundToInt(static_cast<float>(InteractionOutput.Score) * FMath::Max(0.0f, Step->InteractionScoreMultiplier));
		WeightedContributions.Add(WeightedContribution);
	}

	if (WeightedContributions.Num() == 0)
	{
		return 0;
	}

	switch (Recipe->InteractionScoreMode)
	{
	case ERecipeInteractionScoreMode::Average:
	{
		int32 Sum = 0;
		for (const int32 Value : WeightedContributions)
		{
			Sum += Value;
		}
		return FMath::RoundToInt(static_cast<float>(Sum) / static_cast<float>(WeightedContributions.Num()));
	}
	case ERecipeInteractionScoreMode::BestStep:
	{
		int32 BestValue = WeightedContributions[0];
		for (const int32 Value : WeightedContributions)
		{
			BestValue = FMath::Max(BestValue, Value);
		}
		return BestValue;
	}
	case ERecipeInteractionScoreMode::WorstStep:
	{
		int32 WorstValue = WeightedContributions[0];
		for (const int32 Value : WeightedContributions)
		{
			WorstValue = FMath::Min(WorstValue, Value);
		}
		return WorstValue;
	}
	case ERecipeInteractionScoreMode::Additive:
	default:
	{
		int32 Sum = 0;
		for (const int32 Value : WeightedContributions)
		{
			Sum += Value;
		}
		return Sum;
	}
	}
}

int32 URecipeSystem::ComputeFinalScore(const URecipeDataAsset* Recipe, const TArray<FInteractionOutput>& InteractionOutputs, bool bRecipeCompleted) const
{
	return ComputeFinalScoreWithContext(Recipe, InteractionOutputs, bRecipeCompleted, FRecipeScoreContext{});
}

int32 URecipeSystem::ComputeFinalScoreWithContext(const URecipeDataAsset* Recipe, const TArray<FInteractionOutput>& InteractionOutputs, bool bRecipeCompleted, const FRecipeScoreContext& ScoreContext) const
{
	if (Recipe == nullptr)
	{
		return 0;
	}

	int32 TotalScore = 0;
	int32 FailurePenaltyTotal = 0;
	for (int32 StepIndex = 0; StepIndex < Recipe->Steps.Num(); ++StepIndex)
	{
		const UItemTransformation* Step = Recipe->Steps[StepIndex];
		if (Step == nullptr)
		{
			continue;
		}

		TotalScore += Step->BaseStepScore;

		if (InteractionOutputs.IsValidIndex(StepIndex))
		{
			const FInteractionOutput& InteractionOutput = InteractionOutputs[StepIndex];
			if (InteractionOutput.InteractionResult == EInteractionResult::Fail || InteractionOutput.InteractionResult == EInteractionResult::Timeout || InteractionOutput.InteractionResult == EInteractionResult::Cancelled)
			{
				FailurePenaltyTotal += Step->FailurePenalty;
			}
		}
	}

	TotalScore += ComputeInteractionContribution(Recipe, InteractionOutputs);
	TotalScore -= FailurePenaltyTotal;

	if (bRecipeCompleted)
	{
		TotalScore += Recipe->CompletionBonusScore;
	}

	const float ClampedTimeRatio = FMath::Clamp(ScoreContext.TimeRemainingRatio, 0.0f, 1.0f);
	const int32 TimeBonus = FMath::RoundToInt(static_cast<float>(FMath::Max(0, ScoreContext.TimeBonusMax)) * ClampedTimeRatio);
	TotalScore += TimeBonus;

	const float DifficultyMultiplier = FMath::Max(0.0f, ScoreContext.DifficultyMultiplier);
	const float StreakMultiplier = FMath::Max(0.0f, ScoreContext.StreakMultiplier);
	const float ContextMultiplier = DifficultyMultiplier * StreakMultiplier;
	TotalScore = FMath::RoundToInt(static_cast<float>(TotalScore) * ContextMultiplier);

	return FMath::Max(0, TotalScore);
}

bool URecipeSystem::DoesItemMatchStep(const FPrimaryAssetId& ItemId, const UItemTransformation* Step, FText& OutFailureReason, ERecipeValidationError* OutErrorCode) const
{
	auto SetError = [&OutErrorCode](ERecipeValidationError ErrorCode)
	{
		if (OutErrorCode != nullptr)
		{
			*OutErrorCode = ErrorCode;
		}
	};

	if (!ItemId.IsValid())
	{
		SetError(ERecipeValidationError::InvalidInputItemId);
		OutFailureReason = FText::FromString(TEXT("Input item id is invalid."));
		return false;
	}

	if (Step == nullptr)
	{
		SetError(ERecipeValidationError::StepNull);
		OutFailureReason = FText::FromString(TEXT("Step is null."));
		return false;
	}

	if (!Step->HasInputConstraints())
	{
		SetError(ERecipeValidationError::InvalidRecipeDefinition);
		OutFailureReason = FText::FromString(TEXT("Step has no input constraints."));
		return false;
	}

	UItemAsset* InputAsset = ResolveItemAssetFromId(ItemId);
	if (InputAsset == nullptr)
	{
		SetError(ERecipeValidationError::InputAssetResolveFailed);
		OutFailureReason = FText::Format(
			FText::FromString(TEXT("Unable to resolve item asset '{0}'.")),
			FText::FromString(ItemId.ToString()));
		return false;
	}

	if (Step->InputItem != nullptr && Step->InputItem->GetPrimaryAssetId() != ItemId)
	{
		SetError(ERecipeValidationError::InputItemMismatch);
		OutFailureReason = FText::Format(
			FText::FromString(TEXT("Input item '{0}' does not match required item '{1}'.")),
			FText::FromString(ItemId.ToString()),
			FText::FromString(Step->InputItem->GetPrimaryAssetId().ToString()));
		return false;
	}

	if (!ContainsAll(InputAsset->DataTags, Step->RequiredItemDataTags))
	{
		SetError(ERecipeValidationError::MissingItemDataTags);
		OutFailureReason = FText::FromString(TEXT("Input item is missing required data tags."));
		return false;
	}

	if (!ContainsAll(InputAsset->TransformationFlags, Step->RequiredTransformationFlags))
	{
		SetError(ERecipeValidationError::MissingTransformationFlags);
		OutFailureReason = FText::FromString(TEXT("Input item is missing required transformation flags."));
		return false;
	}

	const bool bNeedsIngredientValidation = Step->bRequireProcessedIngredient || Step->RequiredIngredientStateFlags.Num() > 0 || Step->RequiredIngredientStateTags.Num() > 0;
	if (!bNeedsIngredientValidation)
	{
		SetError(ERecipeValidationError::None);
		return true;
	}

	const UIngredientData* IngredientData = Cast<UIngredientData>(InputAsset);
	if (IngredientData == nullptr)
	{
		SetError(ERecipeValidationError::IngredientExpected);
		OutFailureReason = FText::FromString(TEXT("Input item is not an ingredient."));
		return false;
	}

	if (Step->bRequireProcessedIngredient && IngredientData->Type != EIngredientType::Processed)
	{
		SetError(ERecipeValidationError::IngredientNotProcessed);
		OutFailureReason = FText::FromString(TEXT("Input ingredient is not in processed state."));
		return false;
	}

	if (!ContainsAll(IngredientData->StateDescriptor.StateFlags, Step->RequiredIngredientStateFlags))
	{
		SetError(ERecipeValidationError::MissingIngredientStateFlags);
		OutFailureReason = FText::FromString(TEXT("Input ingredient is missing required state flags."));
		return false;
	}

	if (!ContainsAll(IngredientData->StateDescriptor.StateTags, Step->RequiredIngredientStateTags))
	{
		SetError(ERecipeValidationError::MissingIngredientStateTags);
		OutFailureReason = FText::FromString(TEXT("Input ingredient is missing required state tags."));
		return false;
	}

	SetError(ERecipeValidationError::None);
	return true;
}
