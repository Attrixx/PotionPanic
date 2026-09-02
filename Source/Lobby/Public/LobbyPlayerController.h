// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerState.h"
#include "LobbyGameState.h"
#include "LobbyPlayerController.generated.h"

DEFINE_LOG_CATEGORY_STATIC(MS_LobbyPlayerController, Log, All);

class ULobby;
class UUserWidget;
class ULevelSequencePlayer;
class ALevelSequenceActor;
class UDataTable;
class ULocalPlayerRegistrationComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class LOBBY_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ALobbyPlayerController();

protected:

	void BeginPlay() override;
	void SetupInputComponent() override;
	void ReceivedPlayer() override;

public:

	UFUNCTION(BlueprintCallable)
	void TransitionToArea(ECameraPosition TargetArea);
	
	ULevelSequencePlayer* PlaySequence(ELevelSequenceType SequenceType, bool bPlayForward = true);
	ULevelSequencePlayer* PlaySequenceActor(ALevelSequenceActor* SequenceActor, bool bPlayForward = true);

	void SetInLobby(bool bNewInLobby);

	/** Called from a LevelSequence to notify that the settings screen is now visible. */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Settings")
	void ShowSettings();

	/** Hides the settings by playing the appropriate sequence in reverse. Can be called from the UI button or via MenuAction. */
	UFUNCTION(BlueprintCallable, Category = "Lobby|Settings")
	void HideSettings();

protected:

	ECameraPosition CurrentLocalCameraPosition = ECameraPosition::Exterior;

	/** Area the camera was in before entering the settings screen, restored by HideSettings. */
	ECameraPosition CameraPositionBeforeSettings = ECameraPosition::Exterior;

	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> ActiveSequencePlayer = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby|UI")
	TSubclassOf<UUserWidget> LobbyInterfaceWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> LobbyInterfaceWidgetInstance;

	UFUNCTION()
	void OnLobbyInterfaceSequenceFinished();

	UPROPERTY(EditDefaultsOnly, Category = "Lobby|UI")
	TSubclassOf<UUserWidget> SettingsWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> SettingsWidgetInstance;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULocalPlayerRegistrationComponent> LocalPlayerRegistrationComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> BaseInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> LobbyInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LeaveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> InviteAction;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MenuAction;

protected:

	UFUNCTION()
	void Leave(const FInputActionValue& Value);
	UFUNCTION()
	void Invite(const FInputActionValue& Value);
	UFUNCTION()
	void HandleMenuAction(const FInputActionValue& Value);

	UPROPERTY(ReplicatedUsing = OnRep_InLobby)
	bool bInLobby = false;

	UFUNCTION()
	void OnRep_InLobby();

	void PrimaryPlayerLeave();

	bool bInSettings = false;
	
};
