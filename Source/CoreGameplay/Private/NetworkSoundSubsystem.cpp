// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkSoundSubsystem.h"
#include "NetworkSoundRelay.h"

#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogNetworkSound, Log, All);

int32 UNetworkSoundSubsystem::PlayNetworkedSound(USoundBase* Sound, FVector Location, AActor* Instigator)
{
	if (!Sound)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("PlayNetworkedSound: Sound is null, skipping."));
		return -1;
	}

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		return -1;
	}

	// ── 1. Generate a unique handle for this sound instance ──
	const int32 Handle = GenerateHandle(Instigator);

	// ── 2. Play immediately in local at full volume (client-predicted, zero latency) ──
	//       SpawnSoundAtLocation returns the AudioComponent so we can stop it later.
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(World, Sound, Location, FRotator::ZeroRotator, 1.0f);
	if (AudioComp)
	{
		ActiveSounds.Add(Handle, AudioComp);
	}

	// ── 3. Resolve the instigator's PlayerState to exclude them from the Multicast ──
	APlayerState* InstigatorState = nullptr;
	if (Instigator)
	{
		if (APawn* InstigatorPawn = Cast<APawn>(Instigator))
		{
			InstigatorState = InstigatorPawn->GetPlayerState();
		}
	}

	// ── 4. Relay the sound to other clients through the replicated relay actor ──
	// If called directly on the server (no instigating client), InstigatorState will be
	// nullptr and all clients will play the sound — an unlikely case in this project.
	ANetworkSoundRelay* Relay = GetOrCreateRelay();
	if (!Relay)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("PlayNetworkedSound: Could not find or create a NetworkSoundRelay."));
		return Handle;
	}

	Relay->Server_BroadcastSound(Sound, Location, InstigatorState, Handle);
	return Handle;
}

void UNetworkSoundSubsystem::StopNetworkedSound(int32 Handle)
{
	if (Handle == -1)
	{
		return;
	}

	// ── 1. Stop locally ──
	StopRemoteSound(Handle);

	// ── 2. Broadcast the stop to all other clients ──
	ANetworkSoundRelay* Relay = GetOrCreateRelay();
	if (!Relay)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("StopNetworkedSound: Could not find or create a NetworkSoundRelay."));
		return;
	}

	Relay->Server_BroadcastStop(Handle);
}

void UNetworkSoundSubsystem::RegisterRemoteSound(int32 Handle, UAudioComponent* AudioComp)
{
	if (AudioComp)
	{
		ActiveSounds.Add(Handle, AudioComp);
	}
}

void UNetworkSoundSubsystem::StopRemoteSound(int32 Handle)
{
	if (TWeakObjectPtr<UAudioComponent>* Found = ActiveSounds.Find(Handle))
	{
		if (Found->IsValid())
		{
			Found->Get()->Stop();
		}
		ActiveSounds.Remove(Handle);
	}
}

int32 UNetworkSoundSubsystem::GenerateHandle(AActor* Instigator)
{
	// Encode the instigator's PlayerId into the upper part of the handle so that
	// concurrent sounds from different players do not share the same key on remote clients.
	// Format: (PlayerId * 100000) + LocalCounter, assuming fewer than 100 000 sounds per player per session.
	int32 PlayerId = 0;
	if (Instigator)
	{
		if (const APawn* InstigatorPawn = Cast<APawn>(Instigator))
		{
			if (const APlayerState* PS = InstigatorPawn->GetPlayerState())
			{
				PlayerId = PS->GetPlayerId();
			}
		}
	}

	return PlayerId * 100000 + (NextHandle++ % 100000);
}

ANetworkSoundRelay* UNetworkSoundSubsystem::GetOrCreateRelay()
{
	// Return the cached relay if it is still valid.
	if (CachedRelay.IsValid())
	{
		return CachedRelay.Get();
	}

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Look for a relay already present in the world (e.g. after a seamless travel).
	for (TActorIterator<ANetworkSoundRelay> It(World); It; ++It)
	{
		CachedRelay = *It;
		return CachedRelay.Get();
	}

	// No relay found — spawn one.
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANetworkSoundRelay* NewRelay = World->SpawnActor<ANetworkSoundRelay>(
		ANetworkSoundRelay::StaticClass(),
		FTransform::Identity,
		SpawnParams
	);

	if (NewRelay)
	{
		UE_LOG(LogNetworkSound, Log, TEXT("PlayNetworkedSound: Spawned a new ANetworkSoundRelay."));
		CachedRelay = NewRelay;
	}

	return NewRelay;
}
