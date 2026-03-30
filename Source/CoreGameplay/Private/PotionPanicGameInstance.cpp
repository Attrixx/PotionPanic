// Fill out your copyright notice in the Description page of Project Settings.
#include "PotionPanicGameInstance.h"
#include "MultiplayerSessionsSubsystem.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"

void UPotionPanicGameInstance::StartGameInstance()
{
	Super::StartGameInstance();
	MultiplayerSessionsSubsystem = GetSubsystem<UMultiplayerSessionsSubsystem>();

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnSessionUserInviteAccepted.AddUObject(this, &ThisClass::OnAcceptInvite);
	}
}

void UPotionPanicGameInstance::OnJoinSessions(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
			if (SessionInterface.IsValid())
			{
				FString Address;
				if (SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
				{
					APlayerController* PlayerController = GetFirstLocalPlayerController();
					if (PlayerController)
					{
						PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
					}
				}
			}
		}
	}
}

void UPotionPanicGameInstance::OnAcceptInvite(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult& InviteResult)
{
	if (!MultiplayerSessionsSubsystem || !bWasSuccessful) return;

	MultiplayerSessionsSubsystem->JoinSession(InviteResult);
}
