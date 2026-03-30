// Fill out your copyright notice in the Description page of Project Settings.


#include "StartupGameMode.h"
#include "MultiplayerSessionsSubsystem.h"

void AStartupGameMode::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);

		TMap<FName, FVariant> SessionSettings;
		MultiplayerSessionsSubsystem->CreateSession(SessionSettings);
	}
}

void AStartupGameMode::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (UWorld* World = GetWorld())
		{
			World->ServerTravel(FString::Printf(TEXT("%s?listen"), *LobbyMapURL));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to create session"));
	}
}
