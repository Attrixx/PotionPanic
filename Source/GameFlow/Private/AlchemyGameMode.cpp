// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameMode.h"
#include "AlchemyGameState.h"
#include "PotionPanicPlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemyGameMode, Log, All);

AAlchemyGameMode::AAlchemyGameMode()
{
	GameStateClass = AAlchemyGameState::StaticClass();

	// Shared PlayerState so the customization chosen in the Lobby survives seamless travel here.
	PlayerStateClass = APotionPanicPlayerState::StaticClass();

	// Match the lobby: keep the NetDriver alive across level transitions so a listen server can
	// travel between levels without hitting a Steam P2P vport re-Listen failure.
	bUseSeamlessTravel = true;
}

void AAlchemyGameMode::InitGameState()
{
	Super::InitGameState();

	auto* GS = GetGameState<AAlchemyGameState>();
	check(IsValid(GS));

	GS->SetWorldData(GetWorldDataForCurrentWorld());
}

TSoftObjectPtr<UWorldData> AAlchemyGameMode::GetWorldDataForCurrentWorld()
{
	if (!IsValid(GetWorld()))
		return nullptr;

	FString CurrentMapName = GetWorld()->GetMapName();
	CurrentMapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

	for (const auto& Kvp : WorldsData)
	{
		if (Kvp.Key.GetAssetName() == CurrentMapName)
		{
			return Kvp.Value;
		}
	}

	UE_LOGFMT(MS_AlchemyGameMode, Error, "Game mode is missing world data for this world ({0}).", GetWorld()->GetName());
	return nullptr;
}
