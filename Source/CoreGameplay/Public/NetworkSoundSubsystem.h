// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NetworkSoundSubsystem.generated.h"

class USoundBase;
class ANetworkSoundRelay;

/**
 * GameInstance subsystem acting as the single entry point for playing networked sounds.
 *
 * PlayNetworkedSound() behaviour:
 *   1. Plays the sound immediately in local at full volume (client-predicted, zero latency).
 *   2. Sends a Server RPC via ANetworkSoundRelay to relay the sound via NetMulticast.
 *   3. Other clients receive the sound through the Multicast and play it at RemoteVolumeMultiplier.
 *
 * Accessible from anywhere:
 *   UGameInstance* GI = GetGameInstance();
 *   UNetworkSoundSubsystem* SoundSys = GI->GetSubsystem<UNetworkSoundSubsystem>();
 *   SoundSys->PlayNetworkedSound(MySound, Location, InstigatorActor);
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
	 * Can be called from any machine (client or server).
	 * If Instigator is nullptr, the sound will be played at full volume on all machines.
	 *
	 * @param Sound      The sound to play (2D or spatialized depending on its attenuation settings).
	 * @param Location   World position used for spatialized sounds.
	 * @param Instigator The actor that triggered the sound (used to identify the instigating machine).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sound|Network")
	void PlayNetworkedSound(USoundBase* Sound, FVector Location, AActor* Instigator);

private:

	/** Returns the existing relay actor, or spawns a new one if none is found. */
	ANetworkSoundRelay* GetOrCreateRelay();

	/** Cached reference to the relay actor to avoid searching the world on every call. */
	TWeakObjectPtr<ANetworkSoundRelay> CachedRelay;
};
