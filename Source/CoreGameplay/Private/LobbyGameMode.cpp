// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

ALobbyGameMode::ALobbyGameMode()
{
}

void ALobbyGameMode::BeginPlay()
{
}

bool ALobbyGameMode::CanHandleNewPlayer()
{
	UE_LOG(LogTemp, Warning, TEXT("Player count: %d"), PlayerCount);
	return true;
}
