// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameState.h"
#include "WorldData.h"
#include "Rounds/RoundTree.h"
#include "Rounds/RoundLoader.h"
#include <Net/UnrealNetwork.h>

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

float AAlchemyGameState::GetRoundRemainingTime() const
{
	return RoundEndTime - GetServerWorldTimeSeconds();
}

void AAlchemyGameState::OnRep_SoftWorldData()
{
	if (SoftWorldData.IsNull())
	{
		UE_LOGFMT(MS_AlchemyGameState, Error, "Received invalid world data.");
		return;
	}

	SoftWorldData.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateUObject(this, &ThisClass::OnNewWorldDataLoaded));
}

void AAlchemyGameState::OnNewWorldDataLoaded(const FSoftObjectPath& RequestedPath, UObject* InLoadedObject)
{
	if (SoftWorldData.ToSoftObjectPath() != RequestedPath)
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "'{0}' loaded but we were waiting for '{1}'.", SoftWorldData.ToString(), RequestedPath.ToString());
		return;
	}

	auto* NewWorldData = Cast<UWorldData>(InLoadedObject);
	if (!IsValid(NewWorldData))
	{
		UE_LOGFMT(MS_AlchemyGameState, Error, "Loaded invalid world data.");
		return;
	}

	WorldData = NewWorldData;

	if (!HasAuthority())
		return;

	if (URoundTree* Rounds = WorldData->Rounds)
	{
		auto& SetupData = Rounds->GetRoot().Content;
		FOnRoundAppliedDelegate OnRoundApplied;
		OnRoundApplied.BindDynamic(this, &ThisClass::OnRootRoundApplied);
		URoundLoader::LoadAndApplyRound(this, SetupData, OnRoundApplied);
	}
}

void AAlchemyGameState::OnRootRoundApplied()
{
	// TODO: Start match
}
