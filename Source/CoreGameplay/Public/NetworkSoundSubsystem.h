// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetworkSoundSubsystem.generated.h"

class USoundBase;
class UAudioComponent;
class ANetworkSoundRelay;
class APlayerState;
class UNetworkSoundComponent;

/**
 * GameInstance subsystem acting as the single entry point for playing networked sounds.
 *
 * PlayNetworkedSound() behaviour:
 *   1. Plays the sound immediately in local at full volume (client-predicted, zero latency).
 *   2. Routes a Server RPC through UNetworkSoundComponent (on the instigating Pawn) to the server.
 *   3. The server calls BroadcastSoundOnServer(), which triggers a NetMulticast on ANetworkSoundRelay.
 *   4. Other clients receive the sound via the Multicast and play it at RemoteVolumeMultiplier.
 *
 * Why UNetworkSoundComponent instead of a Server RPC on ANetworkSoundRelay?
 *   UE requires Server RPCs to be called on an actor owned by the calling client.
 *   ANetworkSoundRelay is spawned by the server with no owner, so non-host clients cannot
 *   call Server RPCs on it — they are silently dropped. The Pawn is always owned by its
 *   local PlayerController, making Server RPCs on UNetworkSoundComponent valid for all clients.
 *
 * Accessible from anywhere:
 *   UNetworkSoundSubsystem* SoundSys = GetGameInstance()->GetSubsystem<UNetworkSoundSubsystem>();
 *   int32 Handle = SoundSys->PlayNetworkedSound(MySound, Location, InstigatorActor);
 *   SoundSys->StopNetworkedSound(Handle);
 */
UCLASS()
class COREGAMEPLAY_API UNetworkSoundSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	/**
	 * Plays a sound locally at full volume AND broadcasts it over the network at reduced volume
	 * on every other machine.
	 *
	 * Returns a handle that can be passed to StopNetworkedSound() to stop a looping sound.
	 * The handle can be safely ignored for one-shot sounds.
	 *
	 * @param Sound      The sound to play (2D or spatialized depending on its attenuation settings).
	 * @param Location   World position used for spatialized sounds.
	 * @param Instigator The actor that triggered the sound. Must have a UNetworkSoundComponent
	 *                   attached for network broadcasting to work.
	 * @return           A sound handle to use with StopNetworkedSound(). Returns -1 on failure.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sound|Network")
	int32 PlayNetworkedSound(USoundBase* Sound, FVector Location, AActor* Instigator);

	/**
	 * Stops a looping sound on all machines using the handle returned by PlayNetworkedSound().
	 * Safe to call with an invalid handle (no-op).
	 *
	 * @param Handle The handle returned by PlayNetworkedSound().
	 */
	UFUNCTION(BlueprintCallable, Category = "Sound|Network")
	void StopNetworkedSound(int32 Handle);

	/**
	 * Called by UNetworkSoundComponent on the server after its Server_RelaySound RPC fires.
	 * Triggers the NetMulticast on ANetworkSoundRelay.
	 * Not intended for direct use — call PlayNetworkedSound() instead.
	 */
	void BroadcastSoundOnServer(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle);

	/**
	 * Called by UNetworkSoundComponent on the server after its Server_RelayStop RPC fires.
	 * Triggers the NetMulticast stop on ANetworkSoundRelay.
	 * Not intended for direct use — call StopNetworkedSound() instead.
	 */
	void BroadcastStopOnServer(int32 Handle);

	/**
	 * Called by ANetworkSoundRelay on remote clients to register a spawned AudioComponent
	 * so it can be stopped later by StopNetworkedSound().
	 * Not intended for direct use.
	 */
	void RegisterRemoteSound(int32 Handle, UAudioComponent* AudioComp);

	/**
	 * True when this machine already holds a live AudioComponent for Handle, i.e. it originated
	 * the sound through PlayNetworkedSound(). ANetworkSoundRelay uses this to skip its multicast
	 * locally. Not intended for direct use.
	 */
	bool HasActiveSound(int32 Handle) const;

	/**
	 * Called by ANetworkSoundRelay on remote clients to stop a registered AudioComponent.
	 * Not intended for direct use — call StopNetworkedSound() instead.
	 */
	void StopRemoteSound(int32 Handle);

private:

	/** Returns the existing relay actor, or spawns a new one if none is found. */
	ANetworkSoundRelay* GetOrCreateRelay();

	/**
	 * Generates a handle unique across all players in the session.
	 * Encodes the instigator's PlayerId to avoid handle collisions between concurrent players.
	 * Format: (PlayerId * 100000) + LocalCounter, assuming fewer than 100 000 sounds per player per session.
	 */
	int32 GenerateHandle(AActor* Instigator);

	/** Cached reference to the relay actor to avoid searching the world on every call. */
	TWeakObjectPtr<ANetworkSoundRelay> CachedRelay;

	/** Tracks active AudioComponents by handle so they can be stopped on demand. */
	TMap<int32, TWeakObjectPtr<UAudioComponent>> ActiveSounds;

	/**
	 * Tracks the UNetworkSoundComponent responsible for each handle so StopNetworkedSound()
	 * can route the Server RPC through the correct owned actor.
	 */
	TMap<int32, TWeakObjectPtr<UNetworkSoundComponent>> ActiveSoundComponents;

	/** Per-machine counter incremented each time a sound is played. */
	int32 NextHandle = 0;
};
