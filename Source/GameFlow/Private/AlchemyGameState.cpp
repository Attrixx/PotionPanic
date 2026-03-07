// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameState.h"
#include "WorldData.h"
#include "RecipeSystem.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemyGameState, Log, All);

void AAlchemyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAlchemyGameState, SoftWorldData);
}

void AAlchemyGameState::SetWorldData(const TSoftObjectPtr<UWorldData>& NewWorldData)
{
	if (HasAuthority())
	{
		SoftWorldData = NewWorldData;
		OnRep_SoftWorldData();
	}
}

void AAlchemyGameState::OnRep_SoftWorldData()
{
	WorldData = SoftWorldData.LoadSynchronous();
	if (!WorldData)
	{
		UE_LOGFMT(MS_AlchemyGameState, Error, "Received invalid world data.");
		return;
	}

	ConfigureRecipeSystem();
}

void AAlchemyGameState::ConfigureRecipeSystem() const
{
	check(IsValid(WorldData));

	if (auto* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>())
	{
		RecipeSystem->ClearRecipes();
		for (URecipeAsset* Recipe : WorldData->Recipes)
		{
			RecipeSystem->AddRecipe(Recipe);
		}

		UE_LOGFMT(MS_AlchemyGameState,
			Log,
			"Added {0} transformations to the Recipe System, using {1} recipes.",
			RecipeSystem->GetTransformations().Num(),
			WorldData->Recipes.Num());
	}
}
