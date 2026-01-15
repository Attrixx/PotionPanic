// Fill out your copyright notice in the Description page of Project Settings.
#include "PotionPanicGameInstance.h"

void UPotionPanicGameInstance::SavePlayerColor(FUniqueNetIdRepl PlayerId, FColor Color)
{
	if (!PlayerId.IsValid()) return; 
	PlayerSelectedColor.Add(PlayerId, Color); 
}

FColor UPotionPanicGameInstance::GetPlayerColor(FUniqueNetIdRepl PlayerId)
{
	if (PlayerSelectedColor.Contains(PlayerId)) {

		return PlayerSelectedColor[PlayerId];
	}
	return FColor::White;
}
