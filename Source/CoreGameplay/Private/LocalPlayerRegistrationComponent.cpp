// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalPlayerRegistrationComponent.h"
#include "CustomGameViewportClient.h"

#include "Kismet/GameplayStatics.h"

ULocalPlayerRegistrationComponent::ULocalPlayerRegistrationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULocalPlayerRegistrationComponent::HandleLeaveRequest()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());

	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (IsValid(LocalPlayer) && LocalPlayer->GetControllerId() == 0)
	{
		// Primary player leaves: do nothing, PC must handle this itself
		OnPrimaryPlayerRequestLeave.Broadcast();
	}
	else
	{
		// Secondary player leaves
		UGameplayStatics::RemovePlayer(PC, true);
		ServerLocalPlayerLeave();
	}
}


void ULocalPlayerRegistrationComponent::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (PC && PC->IsLocalController())
	{
		ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
		if (IsValid(LocalPlayer) && LocalPlayer->GetControllerId() == 0)
		{
			TObjectPtr<UCustomGameViewportClient> ViewportClient = Cast<UCustomGameViewportClient>(GetWorld()->GetGameViewport());
			if (ViewportClient)
			{
				ViewportClient->OnLocalPlayerJoinRequest.AddDynamic(this, &ThisClass::HandleJoinRequest);
			}
		}
	}
}

void ULocalPlayerRegistrationComponent::HandleJoinRequest(int32 ControllerId)
{
	ServerRequestNewLocalPlayer(ControllerId);
}

void ULocalPlayerRegistrationComponent::ServerRequestNewLocalPlayer_Implementation(int32 ControllerId)
{
	// TODO: Check if we can add a new player
	ClientAuthorizeNewLocalPlayer(ControllerId);
}

void ULocalPlayerRegistrationComponent::ClientAuthorizeNewLocalPlayer_Implementation(int32 ControllerId)
{
	APlayerController* NewPC = UGameplayStatics::CreatePlayer(GetWorld(), ControllerId, true);
}

void ULocalPlayerRegistrationComponent::ServerLocalPlayerLeave_Implementation()
{
	// If this PC shares a connection with other PCs, we should not destroy the connection
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		UChildConnection* ChildConnection = Cast<UChildConnection>(PC->Player);
		if (ChildConnection)
		{
			UNetConnection* parent = ChildConnection->Parent;
			ChildConnection->CleanUp();
			parent->Children.Remove(ChildConnection);
		}
	}
}
