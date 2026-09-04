// Fill out your copyright notice in the Description page of Project Settings.

#include "NetworkSoundSubsystem.h"
#include "NetworkSoundComponent.h"
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

	// ── 1. Generate a handle unique across all players in the session ──
	const int32 Handle = GenerateHandle(Instigator);

	// ── 2. Play immediately in local at full volume (client-predicted, zero latency) ──
	//       SpawnSoundAtLocation returns the AudioComponent so we can stop it later.
	UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
		World, Sound, Location, FRotator::ZeroRotator, 1.0f
	);
	if (AudioComp)
	{
		ActiveSounds.Add(Handle, AudioComp);
	}

	// ── 3. Resolve the instigator's PlayerState (used to skip them in the Multicast) ──
	APlayerState* InstigatorState = nullptr;
	if (const APawn* InstigatorPawn = Cast<APawn>(Instigator))
	{
		InstigatorState = InstigatorPawn->GetPlayerState();
	}

	// ── 4. Route the broadcast through the instigator's UNetworkSoundComponent ──
	// The component lives on the owned Pawn, making its Server RPC valid for any client.
	// If the instigator has no such component (or we're already on the server), fall back
	// to broadcasting directly.
	UNetworkSoundComponent* SoundComp = Instigator
		? Instigator->FindComponentByClass<UNetworkSoundComponent>()
		: nullptr;

	if (SoundComp)
	{
		ActiveSoundComponents.Add(Handle, SoundComp);
		SoundComp->RelaySound(Sound, Location, InstigatorState, Handle);
	}
	else if (World->GetNetMode() != NM_Client)
	{
		// Server or standalone: no need for a Server RPC, broadcast directly.
		BroadcastSoundOnServer(Sound, Location, InstigatorState, Handle);
	}
	else
	{
		UE_LOG(LogNetworkSound, Warning,
			TEXT("PlayNetworkedSound: Instigator '%s' has no UNetworkSoundComponent. "
				 "Sound will not be heard by other clients."),
			Instigator ? *Instigator->GetName() : TEXT("nullptr"));
	}

	return Handle;
}

int32 UNetworkSoundSubsystem::PlayLocalSound(USoundBase* Sound, FVector Location)
{
	if (!Sound)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("PlayLocalSound: Sound is null, skipping."));
		return -1;
	}

	UWorld* World = GetGameInstance()->GetWorld();
	if (!World)
	{
		return -1;
	}

	const int32 Handle = --NextLocalHandle;

	if (UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
			World, Sound, Location, FRotator::ZeroRotator, 1.0f))
	{
		ActiveSounds.Add(Handle, AudioComp);
	}

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

	// A local-only handle never left this machine: there is nothing to relay, and no other machine
	// holds this key.
	if (Handle < 0)
	{
		return;
	}

	// ── 2. Route the stop through the same component that started the sound ──
	if (TWeakObjectPtr<UNetworkSoundComponent>* CompPtr = ActiveSoundComponents.Find(Handle))
	{
		if (CompPtr->IsValid())
		{
			CompPtr->Get()->RelayStop(Handle);
		}
		ActiveSoundComponents.Remove(Handle);
	}
	else
	{
		UWorld* World = GetGameInstance()->GetWorld();
		if (World && World->GetNetMode() != NM_Client)
		{
			BroadcastStopOnServer(Handle);
		}
	}
}

void UNetworkSoundSubsystem::BroadcastSoundOnServer(USoundBase* Sound, FVector Location, APlayerState* InstigatorState, int32 Handle)
{
	ANetworkSoundRelay* Relay = GetOrCreateRelay();
	if (!Relay)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("BroadcastSoundOnServer: Could not find or create a NetworkSoundRelay."));
		return;
	}

	Relay->Multicast_PlaySound(Sound, Location, InstigatorState, Handle);
}

void UNetworkSoundSubsystem::BroadcastStopOnServer(int32 Handle)
{
	ANetworkSoundRelay* Relay = GetOrCreateRelay();
	if (!Relay)
	{
		UE_LOG(LogNetworkSound, Warning, TEXT("BroadcastStopOnServer: Could not find or create a NetworkSoundRelay."));
		return;
	}

	Relay->Multicast_StopSound(Handle);
}

void UNetworkSoundSubsystem::RegisterRemoteSound(int32 Handle, UAudioComponent* AudioComp)
{
	if (!AudioComp)
	{
		return;
	}

	// Never clobber a live entry. The map holds the only reference to the AudioComponent that
	// PlayNetworkedSound() spawned locally; overwriting it leaves that sound playing with nothing
	// able to stop it, which for a looping sound means it never stops at all.
	if (HasActiveSound(Handle))
	{
		AudioComp->Stop();
		return;
	}

	ActiveSounds.Add(Handle, AudioComp);
}

bool UNetworkSoundSubsystem::HasActiveSound(int32 Handle) const
{
	const TWeakObjectPtr<UAudioComponent>* Found = ActiveSounds.Find(Handle);
	return Found && Found->IsValid();
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
	// Format: (PlayerId * 100000) + LocalCounter.
	int32 PlayerId = 0;
	if (const APawn* InstigatorPawn = Cast<APawn>(Instigator))
	{
		if (const APlayerState* PS = InstigatorPawn->GetPlayerState())
		{
			PlayerId = PS->GetPlayerId();
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
		UE_LOG(LogNetworkSound, Log, TEXT("GetOrCreateRelay: Spawned a new ANetworkSoundRelay."));
		CachedRelay = NewRelay;
	}

	return NewRelay;
}
