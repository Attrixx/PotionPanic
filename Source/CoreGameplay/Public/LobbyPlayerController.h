// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerController.generated.h"

class ULobby;
class UInputMappingContext;
class UInputAction;
class ALobbySpawnPoint;
struct FInputActionValue;
class ALobbyPlayerPreview;

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

	UPROPERTY(Replicated, BlueprintReadOnly)
	ALobbyPlayerPreview* MyPreviewActor;
	ALobbySpawnPoint* SpawnPoint;

protected:

	void BeginPlay() override;
	void SetupInputComponent() override;
	void ReceivedPlayer() override;

private:

	UFUNCTION()
	void HandleJoinRequest(int32 ControllerId);

	UFUNCTION(Server, Reliable)
	void ServerRequestNewLocalPlayer();
	UFUNCTION(Client, Reliable)
	void ClientAuthorizeNewLocalPlayer();

	UFUNCTION(Server, Reliable)
	void ServerLocalPlayerLeave();

public:

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetLobbyPlayerColor(FColor NewColor);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Lobby")
	void ClientSwitchMappingContext(bool bInLobby);

private:

	int32 PendingControllerId;
	bool bIsUsingLobbyMappingContext = false;

protected:

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

	void Leave(const FInputActionValue& Value);
	void Invite(const FInputActionValue& Value);
	void HandleMenuAction(const FInputActionValue& Value);
	
};
