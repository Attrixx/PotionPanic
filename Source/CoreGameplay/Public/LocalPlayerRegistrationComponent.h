// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LocalPlayerRegistrationComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnPrimaryPlayerRequestLeave);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COREGAMEPLAY_API ULocalPlayerRegistrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	ULocalPlayerRegistrationComponent();

public:

	void HandleLeaveRequest();

protected:

	virtual void BeginPlay() override;

private:

	UFUNCTION()
	void HandleJoinRequest(int32 ControllerId);

	UFUNCTION(Server, Reliable)
	void ServerRequestNewLocalPlayer(int32 ControllerId);
	UFUNCTION(Client, Reliable)
	void ClientAuthorizeNewLocalPlayer(int32 ControllerId);

	UFUNCTION(Server, Reliable)
	void ServerLocalPlayerLeave();

public:

	FOnPrimaryPlayerRequestLeave OnPrimaryPlayerRequestLeave;

		
};
