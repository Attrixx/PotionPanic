// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
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
#include "Blueprint/UserWidget.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"

ALobbyPlayerController::ALobbyPlayerController()
{
	LocalPlayerRegistrationComponent = CreateDefaultSubobject<ULocalPlayerRegistrationComponent>(TEXT("LocalPlayerRegistrationComponent"));
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

	SetInLobby(false);
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ALobbyPlayerController, bInLobby, COND_OwnerOnly);
}

void ALobbyPlayerController::SetInLobby(bool bNewInLobby)
{
	if (HasAuthority())
	{
		bInLobby = bNewInLobby;
		if (IsLocalController())
		{
			OnRep_InLobby();
		}
	}
}

void ALobbyPlayerController::OnRep_InLobby()
{
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

void ALobbyPlayerController::TransitionToArea(ECameraPosition TargetCameraPosition)
{
	if (CurrentLocalCameraPosition == TargetCameraPosition) return;

	if (LobbyInterfaceWidgetInstance)
	{
		LobbyInterfaceWidgetInstance->RemoveFromParent();
		LobbyInterfaceWidgetInstance = nullptr;
	}

	ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (!LobbyGameState) return;

	ULevelSequencePlayer* PlayedSequence = nullptr;

	for (const FLevelSequenceInfo& SeqInfo : LobbyGameState->GetRegisteredLevelSequences())
	{
		if (SeqInfo.FromArea == CurrentLocalCameraPosition && SeqInfo.ToArea == TargetCameraPosition)
		{
			PlayedSequence = PlaySequenceActor(SeqInfo.SequenceActor, true);
			break;
		}
		else if (SeqInfo.FromArea == TargetCameraPosition && SeqInfo.ToArea == CurrentLocalCameraPosition)
		{
			PlayedSequence = PlaySequenceActor(SeqInfo.SequenceActor, false);
			break;
		}
	}

	if (!PlayedSequence)
	{
		FString CurrentName = StaticEnum<ECameraPosition>()->GetNameStringByValue((int64)CurrentLocalCameraPosition);
		FString TargetName = StaticEnum<ECameraPosition>()->GetNameStringByValue((int64)TargetCameraPosition);

		UE_LOGFMT(MS_LobbyPlayerController, Warning, "No Sequence found in ALobbyGameState for transition from {Current} to {Target}",
			("Current", *CurrentName),
			("Target", *TargetName));
	}
	
	CurrentLocalCameraPosition = TargetCameraPosition;

	if (PlayedSequence)
	{
		PlayedSequence->OnFinished.RemoveAll(this);

		if (TargetCameraPosition == ECameraPosition::LobbyInterface)
		{
			PlayedSequence->OnFinished.AddDynamic(this, &ALobbyPlayerController::OnLobbyInterfaceSequenceFinished);
		}
	}
}

ULevelSequencePlayer* ALobbyPlayerController::PlaySequenceActor(ALevelSequenceActor* SequenceActor, bool bPlayForward)
{
	if (!IsValid(SequenceActor)) return nullptr;
	ULevelSequencePlayer* SequencePlayer = SequenceActor->GetSequencePlayer();
	if (!IsValid(SequencePlayer)) return nullptr;
	if (bPlayForward)
	{
		SequencePlayer->Play();
	}
	else
	{
		SequencePlayer->PlayReverse();
	}
	return SequencePlayer;
}

ULevelSequencePlayer* ALobbyPlayerController::PlaySequence(ELevelSequenceType SequenceType, bool bPlayForward)
{
	ALobbyGameState* LobbyGameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (!IsValid(LobbyGameState)) return nullptr;
	ALevelSequenceActor* SequenceActor = LobbyGameState->GetLevelSequenceActor(SequenceType);
	return PlaySequenceActor(SequenceActor, bPlayForward);
}

void ALobbyPlayerController::OnLobbyInterfaceSequenceFinished()
{
	if (IsLocalController() && LobbyInterfaceWidgetClass)
	{
		if (!LobbyInterfaceWidgetInstance)
		{
			LobbyInterfaceWidgetInstance = CreateWidget<UUserWidget>(this, LobbyInterfaceWidgetClass);
			if (LobbyInterfaceWidgetInstance)
			{
				LobbyInterfaceWidgetInstance->AddToViewport();
			}
		}
	}
}
