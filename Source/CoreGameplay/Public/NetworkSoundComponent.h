// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetworkSoundComponent.generated.h"

class USoundBase;
class APlayerState;

/**
 * Actor component that acts as a Server RPC trampoline for UNetworkSoundSubsystem.
 *
 * UE requires Server RPCs to be called on an actor owned by the calling client's
 * PlayerController. ANetworkSoundRelay is spawned by the server without an owner,
 * so clients cannot call Server RPCs on it directly.
 *
 * This component is attached to the instigating Pawn (e.g. AAlchemistBase), which is
 * always owned by its local PlayerController, making Server RPCs valid from any client.
 *
 * The component forwards the RPC to UNetworkSoundSubsystem on the server, which then
 * triggers the NetMulticast on ANetworkSoundRelay.
 */
UCLASS(ClassGroup = "Sound", meta = (BlueprintSpawnableComponent))
class COREGAMEPLAY_API UNetworkSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UNetworkSoundComponent();

	/**
	 * Sends a sound broadcast request to the server.
	 * Called by UNetworkSoundSubsystem on the instigating client after local playback.
	 */
	void RelaySound(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle);

	/**
	 * Sends a sound stop request to the server.
	 * Called by UNetworkSoundSubsystem on the instigating client.
	 */
	void RelayStop(int32 Handle);

private:

	UFUNCTION(Server, Reliable)
	void Server_RelaySound(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle);

	UFUNCTION(Server, Reliable)
	void Server_RelayStop(int32 Handle);
};
