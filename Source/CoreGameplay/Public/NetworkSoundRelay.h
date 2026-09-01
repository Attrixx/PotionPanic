// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkSoundRelay.generated.h"

class USoundBase;
class APlayerState;

/**
 * Minimal replicated actor whose sole purpose is to relay sounds via NetMulticast.
 * Automatically spawned by UNetworkSoundSubsystem — do not place it manually in the world.
 *
 * Flow:
 *   Instigating client → Server_BroadcastSound (Server RPC)
 *     → Multicast_PlaySound (NetMulticast) → all clients except the instigator
 *                                             play the sound at reduced volume.
 */
UCLASS(NotBlueprintable)
class COREGAMEPLAY_API ANetworkSoundRelay : public AActor
{
	GENERATED_BODY()

public:

	ANetworkSoundRelay();

	/**
	 * Volume multiplier applied on clients that are not the instigator.
	 * Range: 0.0 to 1.0.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Sound", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RemoteVolumeMultiplier = 0.4f;

	/**
	 * Sends a sound request to the server so it can be relayed via Multicast.
	 * Should be called from a client (or the server itself).
	 *
	 * @param Sound           The sound asset to play.
	 * @param Location        World position at which to play the sound.
	 * @param InstigatorState PlayerState of the instigator, used to skip playback on their machine.
	 */
	UFUNCTION(Server, Reliable)
	void Server_BroadcastSound(USoundBase* Sound, FVector Location, APlayerState* InstigatorState);

private:

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySound(USoundBase* Sound, FVector Location, APlayerState* InstigatorState);
};
