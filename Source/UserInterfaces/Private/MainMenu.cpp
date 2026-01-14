// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenu.h"

#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"

void UMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	BT_Host->OnClicked.AddDynamic(this, &UMainMenu::OnHostClicked);
	BT_Join->OnClicked.AddDynamic(this, &UMainMenu::OnJoinClicked);

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMainMenu::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMainMenu::OnJoinSessions);
	}
}

void UMainMenu::OnHostClicked()
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	BT_Host->SetIsEnabled(false);
	TMap<FName, FVariant> SessionSettings;
	MultiplayerSessionsSubsystem->CreateSession(SessionSettings);
}

void UMainMenu::OnJoinClicked()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
	{
		ExternalUI->ShowFriendsUI(0);
	}
}

void UMainMenu::OnCreateSession(bool bWasSuccessful)
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

	BT_Host->SetIsEnabled(true);
}

void UMainMenu::OnJoinSessions(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}
