// Fill out your copyright notice in the Description page of Project Settings.

#include "StationVisualActor.h"

AStationVisualActor::AStationVisualActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);
	bReplicates = false;
}

USceneComponent* AStationVisualActor::GetItemAnchor_Implementation(FName& OutSocketName) const
{
	OutSocketName = NAME_None;
	return GetRootComponent();
}
