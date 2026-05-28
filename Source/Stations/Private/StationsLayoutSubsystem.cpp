// Fill out your copyright notice in the Description page of Project Settings.

#include "StationsLayoutSubsystem.h"
#include "StationsLayoutLayer.h"
#include "StationActor.h"
#include <EngineUtils.h>

void UStationsLayoutSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	GetStationsFromWorld(&InWorld);
}

void UStationsLayoutSubsystem::ApplyLayer(UStationsLayoutLayer* Layer)
{
#if WITH_EDITOR
	if (UWorld* World = GetWorld())
	{
		if (World->WorldType == EWorldType::Editor || World->WorldType == EWorldType::EditorPreview)
		{
			GetStationsFromWorld(World);
		}
	}
#endif

	for (auto It = Stations.CreateIterator(); It; ++It)
	{
		if (AStationActor* Station = It->Get())
		{
			auto& Slot = Station->GetStationSlot();
			UStationAsset* AssetOverride = Layer->GetOverride(Slot);
			Station->SetStationAsset(AssetOverride);
		}
		else
		{
			It.RemoveCurrentSwap();
		}
	}
}

void UStationsLayoutSubsystem::GetStationsFromWorld(UWorld* InWorld)
{
	Stations.Reset();
	for (TActorIterator<AStationActor> It(InWorld); It; ++It)
	{
		if (AStationActor* Station = *It)
		{
			Stations.Add(Station);
		}
	}
}
