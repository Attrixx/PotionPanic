// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkSoundRelay.h"

#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ANetworkSoundRelay::ANetworkSoundRelay()
{
	// This actor does not need to tick.
	PrimaryActorTick.bCanEverTick = false;

	// Replication is required for RPCs to work.
	bReplicates = true;

	// The relay has no visual representation.
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void ANetworkSoundRelay::Server_BroadcastSound_Implementation(USoundBase* Sound, FVector Location, APlayerState* InstigatorState)
{
	if (!Sound)
	{
		return;
	}

	// Relay the sound to all clients. On a dedicated server there is no local player,
	// so Multicast_PlaySound will be a no-op on the server machine itself.
	Multicast_PlaySound(Sound, Location, InstigatorState);
}

void ANetworkSoundRelay::Multicast_PlaySound_Implementation(USoundBase* Sound, FVector Location, APlayerState* InstigatorState)
{
	if (!Sound || !GetWorld())
	{
		return;
	}

	// On a dedicated server there is no local player controller — nothing to play.
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC)
	{
		return;
	}

	// Skip playback if this client is the instigator: the sound was already played locally.
	if (InstigatorState != nullptr && LocalPC->PlayerState == InstigatorState)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, RemoteVolumeMultiplier);
}
