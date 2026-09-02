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
	if (bInSettings)
	{
		HideSettings();
		return;
	}

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

	const ECameraPosition PreviousCameraPosition = CurrentLocalCameraPosition;
	CurrentLocalCameraPosition = TargetCameraPosition;
	if (TargetCameraPosition == ECameraPosition::Settings)
	{
		CameraPositionBeforeSettings = PreviousCameraPosition;
	}

	ALevelSequenceActor* TargetSequenceActor = nullptr;
	bool bPlayForward = true;

	for (const FLevelSequenceInfo& SeqInfo : LobbyGameState->GetRegisteredLevelSequences())
	{
		if (SeqInfo.FromArea == PreviousCameraPosition && SeqInfo.ToArea == TargetCameraPosition)
		{
			TargetSequenceActor = SeqInfo.SequenceActor;
			bPlayForward = true;
			break;
		}
		else if (SeqInfo.FromArea == TargetCameraPosition && SeqInfo.ToArea == PreviousCameraPosition)
		{
			TargetSequenceActor = SeqInfo.SequenceActor;
			bPlayForward = false;
			break;
		}
	}

	ULevelSequencePlayer* NextSequencePlayer = IsValid(TargetSequenceActor) ? TargetSequenceActor->GetSequencePlayer() : nullptr;

	if (IsValid(ActiveSequencePlayer) && ActiveSequencePlayer->IsPlaying())
	{
		if (ActiveSequencePlayer != NextSequencePlayer)
		{
			FQualifiedFrameTime EndTime = ActiveSequencePlayer->GetEndTime();
			FMovieSceneSequencePlaybackParams PlaybackParams;
			PlaybackParams.Frame = EndTime.Time;
			PlaybackParams.UpdateMethod = EUpdatePositionMethod::Jump;
			ActiveSequencePlayer->SetPlaybackPosition(PlaybackParams);
		}
		else
		{
			ActiveSequencePlayer->Pause();
		}
		ActiveSequencePlayer->OnFinished.RemoveAll(this);
	}

	if (TargetSequenceActor)
	{
		ActiveSequencePlayer = PlaySequenceActor(TargetSequenceActor, bPlayForward);
	}

	if (ActiveSequencePlayer)
	{
		ActiveSequencePlayer->OnFinished.RemoveAll(this);

		if (TargetCameraPosition == ECameraPosition::LobbyInterface)
		{
			ActiveSequencePlayer->OnFinished.AddDynamic(this, &ALobbyPlayerController::OnLobbyInterfaceSequenceFinished);
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

void ALobbyPlayerController::ShowSettings()
{
	if (bInSettings) return;

	bInSettings = true;

	if (IsLocalController() && SettingsWidgetClass && !SettingsWidgetInstance)
	{
		SettingsWidgetInstance = CreateWidget<UUserWidget>(this, SettingsWidgetClass);
		if (SettingsWidgetInstance)
		{
			// Bind HideSettings to the widget's OnBackClicked delegate via reflection,
			// avoiding a hard compile-time dependency on UserInterfaces (circular dependency).
			if (FMulticastDelegateProperty* DelegateProp = CastField<FMulticastDelegateProperty>(
				SettingsWidgetInstance->GetClass()->FindPropertyByName(TEXT("OnBackClicked"))))
			{
				FScriptDelegate Delegate;
				Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(ALobbyPlayerController, HideSettings));
				DelegateProp->AddDelegate(Delegate, SettingsWidgetInstance);
			}

			SettingsWidgetInstance->AddToViewport();

			bShowMouseCursor = true;
			// Set input mode to game and UI to allow interaction with the settings widget while still allowing game input.
			FInputModeGameAndUI InputMode;
			SetInputMode(InputMode);
		}
	}
}

void ALobbyPlayerController::HideSettings()
{
	if (!bInSettings) return;

	bInSettings = false;

	if (SettingsWidgetInstance)
	{
		SettingsWidgetInstance->RemoveFromParent();
		SettingsWidgetInstance = nullptr;
	}

	bShowMouseCursor = false;
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	// Route back through TransitionToArea so CurrentLocalCameraPosition, the active
	// sequence and its playback direction stay consistent. It resolves the registered
	// <area> -> Settings sequence and plays it in reverse.
	TransitionToArea(CameraPositionBeforeSettings);
}
