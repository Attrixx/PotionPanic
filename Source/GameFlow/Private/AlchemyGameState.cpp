// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameState.h"
#include "AlchemyWorldSettings.h"
#include "RecipeSystem.h"

void AAlchemyGameState::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* WorldSettings = CastChecked<AAlchemyWorldSettings>(GetWorldSettings()))
	{
		InitializeSubsystems(*WorldSettings);
	}
}

void AAlchemyGameState::InitializeSubsystems(AAlchemyWorldSettings& WorldSettings)
{
	if (auto* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>())
	{
		RecipeSystem->SetRecipes(WorldSettings.RecipeAsset);
	}
}
