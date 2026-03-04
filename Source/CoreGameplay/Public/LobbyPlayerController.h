// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerController.generated.h"

class ULobby;
class ULocalPlayerRegistrationComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ALobbyCharacter;

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ALobbyPlayerController();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	void BeginPlay() override;
	void SetupInputComponent() override;
	void ReceivedPlayer() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetLobbyPlayerColor(FColor NewColor);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Lobby")
	void ClientSwitchMappingContext(bool bInLobby);

	ALobbyCharacter* GetPreviewActor() const { return PreviewActor; }
	void SetPreviewActor(ALobbyCharacter* NewPreviewActor) { PreviewActor = NewPreviewActor; };

private:

	int32 PendingControllerId;
	bool bIsUsingLobbyMappingContext = false;

protected:

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

	UPROPERTY(Replicated, BlueprintReadOnly)
	TObjectPtr<ALobbyCharacter> PreviewActor;

protected:

	void Leave(const FInputActionValue& Value);
	void Invite(const FInputActionValue& Value);
	void HandleMenuAction(const FInputActionValue& Value);

	void PrimaryPlayerLeave();
	
};
