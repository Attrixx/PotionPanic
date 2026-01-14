// Fill out your copyright notice in the Description page of Project Settings.

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() : CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnCreateSessionComplete)),
                                                                 FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete)),
                                                                 JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete)),
                                                                 DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete)),
                                                                 StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnStartSessionComplete))
{
  if (const IOnlineSubsystem *Subsystem = Online::GetSubsystem(UObject::GetWorld()))
  {
    SessionInterface = Subsystem->GetSessionInterface();
  }

  if (SessionInterface)
  {
    SessionInterface->OnSessionUserInviteAcceptedDelegates.AddUObject(this, &UMultiplayerSessionsSubsystem::OnUserInviteAccepted);
  }
}

void UMultiplayerSessionsSubsystem::CreateSession(const TMap<FName, FVariant> &Settings)
{
  if (!SessionInterface.IsValid())
  {
    return;
  }

  auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
  if (ExistingSession != nullptr)
  {
    bCreateSessionOnDestroy = true;
    LastSessionSettingsMap = Settings;

    DestroySession();
    return;
  }

  CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

  LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
  LastSessionSettings->bIsLANMatch = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL" ? true : false;
  
  int32 MaxPlayers = 5;
  if (Settings.Contains(FName("MaxPlayers")) && Settings[FName("MaxPlayers")].GetType() == EVariantTypes::Int32)
  {
      MaxPlayers = Settings[FName("MaxPlayers")].GetValue<int32>();
  }

  bool bPrivateGame = true;
  if (Settings.Contains(FName("bPrivateGame")) && Settings[FName("bPrivateGame")].GetType() == EVariantTypes::Bool)
  {
      bPrivateGame = Settings[FName("bPrivateGame")].GetValue<bool>();
  }

  if (bPrivateGame)
  {
    LastSessionSettings->NumPrivateConnections = MaxPlayers;
    LastSessionSettings->NumPublicConnections = 0;
  }
  else
  {
    LastSessionSettings->NumPublicConnections = MaxPlayers;
  }
  LastSessionSettings->bAllowJoinInProgress = true;
  LastSessionSettings->bAllowJoinViaPresence = true;
  LastSessionSettings->bShouldAdvertise = true;
  LastSessionSettings->bUsesPresence = true;
  LastSessionSettings->bAllowInvites = true;
  
  for (const auto& Entry : Settings)
  {
      switch (Entry.Value.GetType())
      {
      case EVariantTypes::Bool:
          LastSessionSettings->Set(Entry.Key, Entry.Value.GetValue<bool>(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
          break;
      case EVariantTypes::Int32:
          LastSessionSettings->Set(Entry.Key, Entry.Value.GetValue<int32>(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
          break;
      case EVariantTypes::String:
          LastSessionSettings->Set(Entry.Key, Entry.Value.GetValue<FString>(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
          break;
      case EVariantTypes::UInt8:
          LastSessionSettings->Set(Entry.Key, (int32)Entry.Value.GetValue<uint8>(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
          break;
      default:
          break;
      }
  }

  LastSessionSettings->BuildUniqueId = 1;
  LastSessionSettings->bUseLobbiesIfAvailable = true;

  const ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
  {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

    MultiplayerOnCreateSessionComplete.Broadcast(false);
  }
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
  if (!SessionInterface.IsValid())
  {
    return;
  }

  FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

  LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
  LastSessionSearch->MaxSearchResults = MaxSearchResults;
  LastSessionSearch->bIsLanQuery = Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL" ? true : false;
  LastSessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

  const ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
  {
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

    MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
  }
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult &SessionResult)
{
  if (!SessionInterface.IsValid())
  {
    MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
    return;
  }

  JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

  const ULocalPlayer *LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
  if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
  {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

    MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
  }
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
  if (!SessionInterface.IsValid())
  {
    MultiplayerOnDestroySessionComplete.Broadcast(false);
    return;
  }

  DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

  if (!SessionInterface->DestroySession(NAME_GameSession))
  {
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
    MultiplayerOnDestroySessionComplete.Broadcast(false);
  }
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
  }

  MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
  }

  if (LastSessionSearch->SearchResults.Num() <= 0)
  {
    MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
    return;
  }

  MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
  }

  MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
  if (SessionInterface)
  {
    SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
  }
  if (bWasSuccessful && bCreateSessionOnDestroy)
  {
    bCreateSessionOnDestroy = false;
    CreateSession(LastSessionSettingsMap);
  }
  MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnUserInviteAccepted(const bool bWasSuccessful, const int32 ControllerId, FUniqueNetIdPtr UserId, const FOnlineSessionSearchResult &InviteResult)
{
  MultiplayerOnSessionUserInviteAccepted.Broadcast(bWasSuccessful, ControllerId, UserId, InviteResult);
}
