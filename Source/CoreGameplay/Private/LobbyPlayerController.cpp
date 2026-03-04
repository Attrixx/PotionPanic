// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "LobbyCharacter.h"
#include "CustomGameViewportClient.h"
#include "LocalPlayerRegistrationComponent.h"

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
	LocalPlayerRegistrationComponent = CreateDefaultSubobject<ULocalPlayerRegistrationComponent>(TEXT("LocalPlayerRegistrationComponent"));
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyPlayerController, PreviewActor);
}
void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	LocalPlayerRegistrationComponent->OnPrimaryPlayerRequestLeave.AddUObject(this, &ThisClass::PrimaryPlayerLeave);
}

void ALobbyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
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

	ClientSwitchMappingContext(false);
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
	bIsUsingLobbyMappingContext = bInLobby;

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (bInLobby)
		{
			if (Subsystem->HasMappingContext(BaseInputMappingContext))
			{
				Subsystem->RemoveMappingContext(BaseInputMappingContext);
			}
			Subsystem->AddMappingContext(LobbyInputMappingContext, 0);
		}
		else
		{
			if (Subsystem->HasMappingContext(LobbyInputMappingContext))
			{
				Subsystem->RemoveMappingContext(LobbyInputMappingContext);
			}
			Subsystem->AddMappingContext(BaseInputMappingContext, 0);
		}
	}
}

void ALobbyPlayerController::Leave(const FInputActionValue& Value)
{
	LocalPlayerRegistrationComponent->HandleLeaveRequest();
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

void ALobbyPlayerController::PrimaryPlayerLeave()
{
	// exit invite/customize area if host, or leave lobby if not host
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
