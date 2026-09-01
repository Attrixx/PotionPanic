// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkSoundSubsystem.h"
#include "NetworkSoundRelay.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogNetworkSound, Log, All);

void UNetworkSoundSubsystem::PlayNetworkedSound(USoundBase* Sound, FVector Location, AActor* Instigator)
{
	if (!Sound)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("PlayNetworkedSound: Sound is null, skipping."));
		return;
	}

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		return;
	}

	// ── 1. Play immediately in local at full volume (client-predicted, zero latency) ──
	UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, 1.0f);

	// ── 2. Resolve the instigator's PlayerState to exclude them from the Multicast ──
	APlayerState* InstigatorState = nullptr;
	if (Instigator)
	{
		if (APawn* InstigatorPawn = Cast<APawn>(Instigator))
		{
			InstigatorState = InstigatorPawn->GetPlayerState();
		}
	}

	// ── 3. Relay the sound to other clients through the replicated relay actor ──
	// If called directly on the server (no instigating client), InstigatorState will be
	// nullptr and all clients will play the sound — an unlikely case in this project.
	ANetworkSoundRelay* Relay = GetOrCreateRelay();
	if (!Relay)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("PlayNetworkedSound: Could not find or create a NetworkSoundRelay."));
		return;
	}

	Relay->Server_BroadcastSound(Sound, Location, InstigatorState);
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
