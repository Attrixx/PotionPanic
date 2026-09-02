// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkSoundRelay.generated.h"

class USoundBase;
class APlayerState;

/**
 * Minimal replicated actor whose sole purpose is to execute NetMulticast RPCs.
 * Automatically spawned by UNetworkSoundSubsystem — do not place it manually in the world.
 *
 * This actor intentionally has NO Server RPCs. Server RPCs require the calling client to
 * own the actor, which is not possible for a server-spawned actor with no owner. Instead,
 * UNetworkSoundComponent (attached to the instigating Pawn) handles the Server RPC leg,
 * then calls BroadcastSoundOnServer / BroadcastStopOnServer on UNetworkSoundSubsystem,
 * which invokes the Multicast methods below directly on the server.
 *
 * Play flow:
 *   Client → UNetworkSoundComponent::Server_RelaySound (Server RPC, on owned Pawn)
 *     → UNetworkSoundSubsystem::BroadcastSoundOnServer
 *       → ANetworkSoundRelay::Multicast_PlaySound (NetMulticast to all clients)
 *
 * Stop flow:
 *   Client → UNetworkSoundComponent::Server_RelayStop (Server RPC, on owned Pawn)
 *     → UNetworkSoundSubsystem::BroadcastStopOnServer
 *       → ANetworkSoundRelay::Multicast_StopSound (NetMulticast to all clients)
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

	// Called directly by UNetworkSoundSubsystem on the server.
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlaySound(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle);

	// Reliable: a dropped stop packet would leave a looping sound running indefinitely.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopSound(int32 Handle);
};
