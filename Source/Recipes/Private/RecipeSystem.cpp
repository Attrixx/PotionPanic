// Fill out your copyright notice in the Description page of Project Settings.

#include "RecipeSystem.h"

#include "ActivityStepSettings.h"
#include "RecipeAsset.h"
#include "ItemTransformation.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RecipeSystem, Log, All);

void URecipeSystem::AddRecipe(URecipeAsset* Recipe)
{
	if (!Recipe)
		return;

	for (UItemTransformation* Step : Recipe->Steps)
	{
		check(Step);
		Transformations.AddUnique(Step);
	}
}

void URecipeSystem::ClearRecipes()
{
	Transformations.Empty();
}

TOptional<FInstruction> URecipeSystem::CreateInstruction(const FGameplayTagContainer& Tags)
{
	for (UItemTransformation* Transformation : Transformations)
	{
		check(Transformation);
		if (Tags.HasAll(Transformation->InputTags))
		{
			// Returns the first transformation that matches
			return CreateInstruction(*Transformation);
		}
	}
	return NullOpt;
}

FInstruction URecipeSystem::CreateInstruction(UItemTransformation& Transformation)
{
	FInstruction Instruction;
	Instruction.Steps.Empty(Transformation.ActivitySteps.Num());
	for (UActivityStepSettings* Settings : Transformation.ActivitySteps)
	{
		check(Settings);
		Instruction.Steps.Add(Settings->CreateStep(this));
	}
	Instruction.OutputItem = Transformation.OutputItem;
	return Instruction;
}
