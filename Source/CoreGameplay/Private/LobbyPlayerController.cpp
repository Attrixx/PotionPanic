// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerPreview.h"
#include "CustomGameViewportClient.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Net/UnrealNetwork.h"

ALobbyPlayerController::ALobbyPlayerController()
{
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyPlayerController, MyPreviewActor);
}
void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<UCustomGameViewportClient> ViewportClient = Cast<UCustomGameViewportClient>(GetWorld()->GetGameViewport());
	if (ViewportClient)
	{
		ViewportClient->OnLocalPlayerJoinRequest.AddDynamic(this, &ALobbyPlayerController::HandleJoinRequest);
	}
}

void ALobbyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	ClientSwitchMappingContext(false);
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(LeaveAction, ETriggerEvent::Started, this, &ThisClass::Leave);
		EnhancedInputComponent->BindAction(InviteAction, ETriggerEvent::Started, this, &ThisClass::Invite);
		EnhancedInputComponent->BindAction(MenuAction, ETriggerEvent::Started, this, &ThisClass::HandleMenuAction);
	}
}

void ALobbyPlayerController::HandleJoinRequest(int32 ControllerId)
{
	if (!bIsUsingLobbyMappingContext) return;

	// TODO: Handle two request at the same time
	PendingControllerId = ControllerId;
	ServerRequestNewLocalPlayer();
}

void ALobbyPlayerController::ServerRequestNewLocalPlayer_Implementation()
{
	TObjectPtr<ALobbyGameMode> LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode && LobbyGameMode->CanHandleNewPlayer())
	{
		ClientAuthorizeNewLocalPlayer();
	}
}

void ALobbyPlayerController::ClientAuthorizeNewLocalPlayer_Implementation()
{
	APlayerController* NewPC = UGameplayStatics::CreatePlayer(this->GetWorld(), PendingControllerId, true);
}

void ALobbyPlayerController::ServerLocalPlayerLeave_Implementation()
{
	// If this PC shares a connection with other PCs, we should not destroy the connection
	UChildConnection* C = Cast<UChildConnection>(Player);
	if (C)
	{
		UNetConnection* parent = C->Parent;
		C->CleanUp();
		parent->Children.Remove(C);
	}
}

void ALobbyPlayerController::SetLobbyPlayerColor(FColor NewColor)
{
	if (ALobbyPlayerState* LobbyPS = GetPlayerState<ALobbyPlayerState>())
	{
		LobbyPS->SetPlayerColor(NewColor);
	}
}

void ALobbyPlayerController::ClientSwitchMappingContext_Implementation(bool bInLobby)
{
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (bInLobby)
		{
			if (Subsystem->HasMappingContext(BaseInputMappingContext))
			{
				Subsystem->RemoveMappingContext(BaseInputMappingContext);
			}
			bIsUsingLobbyMappingContext = true;
			Subsystem->AddMappingContext(LobbyInputMappingContext, 0);
		}
		else
		{
			if (Subsystem->HasMappingContext(LobbyInputMappingContext))
			{
				Subsystem->RemoveMappingContext(LobbyInputMappingContext);
			}
			bIsUsingLobbyMappingContext = false;
			Subsystem->AddMappingContext(BaseInputMappingContext, 0);
		}
	}
}

void ALobbyPlayerController::Leave(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Leave action triggered"));
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (IsValid(LocalPlayer) && LocalPlayer->GetControllerId() == 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Primary player leaving"));
		// Primary player leaves: exit invite/customize area if host, or leave lobby if not host
		ALobbyPlayerState* PS = GetPlayerState<ALobbyPlayerState>();
		if (!IsValid(PS)) return;

		if (PS->IsHost())
		{
			if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
			{
				LobbyGameMode->RequestLeaveInviteArea(this);
			}	
		}
		else
		{
			// TODO: Get Level name from somewhere
			UGameplayStatics::OpenLevel(this, FName("MainMenu"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Secondary player leaving"));
		// Secondary player leaves
		UGameplayStatics::RemovePlayer(this, true);
		ServerLocalPlayerLeave();
	}
}

void ALobbyPlayerController::Invite(const FInputActionValue& Value)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
	{
		ExternalUI->ShowFriendsUI(0);
	}
}

void ALobbyPlayerController::HandleMenuAction(const FInputActionValue& Value)
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (IsValid(LocalPlayer) && LocalPlayer->GetControllerId() == 0)
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
	}
}
