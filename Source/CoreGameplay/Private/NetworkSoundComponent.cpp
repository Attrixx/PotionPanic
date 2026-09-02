// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkSoundComponent.h"
#include "NetworkSoundSubsystem.h"

#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"

UNetworkSoundComponent::UNetworkSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNetworkSoundComponent::RelaySound(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle)
{
	// Calling a Server RPC on this component works because it is attached to a Pawn
	// that is owned by the local PlayerController — unlike ANetworkSoundRelay which
	// has no owner and therefore rejects Server RPCs from non-host clients.
	Server_RelaySound(Sound, Location, InstigatorState, Handle);
}

void UNetworkSoundComponent::Server_RelaySound_Implementation(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle)
{
	// We are now on the server. Delegate to the subsystem to trigger the NetMulticast.
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UGameInstance* GI = Owner->GetGameInstance();
	if (UNetworkSoundSubsystem* SoundSys = GI ? GI->GetSubsystem<UNetworkSoundSubsystem>() : nullptr)
	{
		SoundSys->BroadcastSoundOnServer(Sound, Location, InstigatorState, Handle);
	}
}

void UNetworkSoundComponent::RelayStop(int32 Handle)
{
	Server_RelayStop(Handle);
}

void UNetworkSoundComponent::Server_RelayStop_Implementation(int32 Handle)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UGameInstance* GI = Owner->GetGameInstance();
	if (UNetworkSoundSubsystem* SoundSys = GI ? GI->GetSubsystem<UNetworkSoundSubsystem>() : nullptr)
	{
		SoundSys->BroadcastStopOnServer(Handle);
	}
}
