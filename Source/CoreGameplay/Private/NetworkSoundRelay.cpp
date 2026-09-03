// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkSoundRelay.h"
#include "NetworkSoundSubsystem.h"

#include "Components/AudioComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ANetworkSoundRelay::ANetworkSoundRelay()
{
	// This actor does not need to tick.
	PrimaryActorTick.bCanEverTick = false;

	// Replication is required for NetMulticast RPCs to work.
	bReplicates = true;

	// The relay has no visual representation.
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void ANetworkSoundRelay::Multicast_PlaySound_Implementation(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle)
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

	// Spawn the sound so we get an AudioComponent back, allowing it to be stopped later.
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
		this, Sound, Location, FRotator::ZeroRotator, RemoteVolumeMultiplier
	);

	// Register the component with the subsystem so StopNetworkedSound() can reach it.
	if (AudioComp)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UNetworkSoundSubsystem* SoundSys = GI->GetSubsystem<UNetworkSoundSubsystem>())
			{
				SoundSys->RegisterRemoteSound(Handle, AudioComp);
			}
		}
	}
}

void ANetworkSoundRelay::Multicast_StopSound_Implementation(int32 Handle)
{
	if (!GetWorld())
	{
		return;
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UNetworkSoundSubsystem* SoundSys = GI->GetSubsystem<UNetworkSoundSubsystem>())
		{
			SoundSys->StopRemoteSound(Handle);
		}
	}
}
