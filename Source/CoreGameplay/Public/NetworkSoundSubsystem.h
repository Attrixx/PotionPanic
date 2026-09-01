// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetworkSoundSubsystem.generated.h"

class USoundBase;
class UAudioComponent;
class ANetworkSoundRelay;
class APlayerState;

/**
 * GameInstance subsystem acting as the single entry point for playing networked sounds.
 *
 * PlayNetworkedSound() behaviour:
 *   1. Plays the sound immediately in local at full volume (client-predicted, zero latency).
 *   2. Sends a Server RPC via ANetworkSoundRelay to relay the sound via NetMulticast.
 *   3. Other clients receive the sound through the Multicast and play it at RemoteVolumeMultiplier.
 *
 * StopNetworkedSound() stops a looping sound on all machines using the handle
 * returned by PlayNetworkedSound().
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
	 * @param Instigator The actor that triggered the sound (used to identify the instigating machine).
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
	 * Called by ANetworkSoundRelay on remote clients to register a spawned AudioComponent
	 * so it can be stopped later by StopNetworkedSound().
	 * Not intended for direct use — call PlayNetworkedSound() instead.
	 */
	void RegisterRemoteSound(int32 Handle, UAudioComponent* AudioComp);

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
	 * Format: (PlayerId * 100000) + LocalCounter.
	 */
	int32 GenerateHandle(AActor* Instigator);

	/** Cached reference to the relay actor to avoid searching the world on every call. */
	TWeakObjectPtr<ANetworkSoundRelay> CachedRelay;

	/** Tracks active AudioComponents by handle so they can be stopped on demand. */
	TMap<int32, TWeakObjectPtr<UAudioComponent>> ActiveSounds;

	/** Per-machine counter incremented each time a sound is played. */
	int32 NextHandle = 0;
};
