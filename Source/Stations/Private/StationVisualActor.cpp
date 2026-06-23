// Fill out your copyright notice in the Description page of Project Settings.

#include "StationVisualActor.h"

USceneComponent* AStationVisualActor::GetItemAnchor_Implementation(FName& OutSocketName) const
{
	OutSocketName = NAME_None;
	return GetRootComponent();
}
