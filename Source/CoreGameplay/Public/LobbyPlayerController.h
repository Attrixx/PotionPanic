// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
class ALobbyPlayerPreview;
UCLASS()
class COREGAMEPLAY_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	ALobbyPlayerController();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly)
	ALobbyPlayerPreview* MyPreviewActor;
protected:

	void BeginPlay() override;
	void SetupInputComponent() override;

private:

	UFUNCTION()
	void HandleJoinRequest(int32 ControllerId);

	UFUNCTION(Server, Reliable)
	void ServerRequestNewLocalPlayer();
	UFUNCTION(Client, Reliable)
	void ClientAuthorizeNewLocalPlayer();

private:

	int32 PendingControllerId;
	
};
