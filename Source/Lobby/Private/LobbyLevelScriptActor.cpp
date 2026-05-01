// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyLevelScriptActor.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"
#include "LobbyAreaTriggerBox.h"
#include "AlchemistBase.h"

#include "Engine/TriggerBox.h"
#include "Components/ShapeComponent.h"
#include "Kismet/GameplayStatics.h"

void ALobbyLevelScriptActor::BeginPlay()
{
	Super::BeginPlay();
	ToggleShowPlayerPreviews(false);
}

void ALobbyLevelScriptActor::RegisterTriggerBoxes(const TMap<ECameraPosition, ATriggerBox*>& TriggerBoxes)
{
	RegisteredTriggerBoxes = TriggerBoxes;

	for (const auto& Pair : RegisteredTriggerBoxes)
	{
		if (IsValid(Pair.Value))
		{
			Pair.Value->OnActorBeginOverlap.AddDynamic(this, &ThisClass::OnTriggerBoxBeginOverlap);
			Pair.Value->OnActorEndOverlap.AddDynamic(this, &ThisClass::OnTriggerBoxEndOverlap);
		}
	}
}

void ALobbyLevelScriptActor::ToggleShowPlayerPreviews(bool bShow)
{
	ALobbyGameState* GameState = GetWorld()->GetGameState<ALobbyGameState>();
	if (!IsValid(GameState)) return;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		ALobbyPlayerState* LobbyPS = Cast<ALobbyPlayerState>(PS);
		if (!IsValid(LobbyPS)) continue;

		if (ACharacter* PlayerCharacter = Cast<ACharacter>(LobbyPS->GetPawn()))
		{
			PlayerCharacter->SetActorHiddenInGame(bShow);
		}
		if (ACharacter* PreviewCharacter = LobbyPS->GetPreviewActor())
		{
			PreviewCharacter->SetActorHiddenInGame(!bShow);
		}
	}
}

void ALobbyLevelScriptActor::OnStartupSequenceFinished()
{
	APlayerController* LocalPC = UGameplayStatics::GetPlayerController(this, 0);
	if (!IsValid(LocalPC)) return;
	ALobbyPlayerState* PlayerState = LocalPC->GetPlayerState<ALobbyPlayerState>();
	if (!IsValid(PlayerState)) return;
	PlayerState->Server_OnStartupSequenceFinished();
}

void ALobbyLevelScriptActor::OnTriggerBoxBeginOverlap(AActor* TriggerBox, AActor* OtherActor)
{
	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	ALobbyAreaTriggerBox* AreaTrigger = Cast<ALobbyAreaTriggerBox>(TriggerBox);
	if (!IsValid(OverlappingCharacter) || !IsValid(AreaTrigger)) return;

	ECameraPosition AreaType = AreaTrigger->GetAreaType();

	// Client Side
	if (OverlappingCharacter->IsLocallyControlled())
	{
		if (ALobbyPlayerController* LobbyPC = Cast<ALobbyPlayerController>(OverlappingCharacter->GetController()))
		{
			LobbyPC->TransitionToArea(AreaType);
		}
	}

	// Server side
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (!IsValid(LobbyGameMode)) return;

	LobbyGameMode->OnPlayerEnterArea(OverlappingCharacter, AreaType);
}

void ALobbyLevelScriptActor::OnTriggerBoxEndOverlap(AActor* TriggerBox, AActor* OtherActor)
{
	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	ALobbyAreaTriggerBox* AreaTrigger = Cast<ALobbyAreaTriggerBox>(TriggerBox);
	if (!IsValid(OverlappingCharacter) || !IsValid(AreaTrigger)) return;

	ECameraPosition AreaType = AreaTrigger->GetAreaType();

	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (!IsValid(LobbyGameMode)) return;

	if (AreaType == ECameraPosition::Entrance)
	{
		LobbyGameMode->OnPlayerLeaveArea(
			OverlappingCharacter, 
			ECameraPosition::Entrance, 
			IsAnyActorInTriggerBox(AActor::StaticClass(), ECameraPosition::Entrance),
			IsActorInTriggerBox(OverlappingCharacter, ECameraPosition::Interior)
		);
	}
	else
	{
		LobbyGameMode->OnPlayerLeaveArea(OverlappingCharacter, AreaType);
	}
}

bool ALobbyLevelScriptActor::IsAnyActorInTriggerBox(TSubclassOf<AActor> ClassToSearch, ECameraPosition TriggerBoxType) const
{
	if (!IsValid(ClassToSearch)) return false;

	if (RegisteredTriggerBoxes.Contains(TriggerBoxType))
	{
		TArray<AActor*> OverlappingActors;
		RegisteredTriggerBoxes[TriggerBoxType]->GetOverlappingActors(OverlappingActors, ClassToSearch);
		return OverlappingActors.Num() > 0;
	}
	else
	{
		FString BoxType = StaticEnum<ECameraPosition>()->GetNameStringByValue((int64)TriggerBoxType);
		UE_LOGFMT(MS_LobbyLevelScriptActor, Warning, "The trigger box {0} is not registered. (Register it in Editor via ALobbyLevelScriptActor::RegisterTriggerBoxes)", BoxType);
	}
	return false;
}

bool ALobbyLevelScriptActor::IsActorInTriggerBox(AActor* Actor, ECameraPosition TriggerBoxType) const
{
	if (!IsValid(Actor)) return false;

	if (RegisteredTriggerBoxes.Contains(TriggerBoxType))
	{
		TArray<AActor*> OverlappingActors;
		RegisteredTriggerBoxes[TriggerBoxType]->GetOverlappingActors(OverlappingActors, Actor->GetClass());
		return OverlappingActors.Contains(Actor);
	}
	else
	{
		FString BoxType = StaticEnum<ECameraPosition>()->GetNameStringByValue((int64)TriggerBoxType);
		UE_LOGFMT(MS_LobbyLevelScriptActor, Warning, "The trigger box {0} is not registered. (Register it in Editor via ALobbyLevelScriptActor::RegisterTriggerBoxes)", BoxType);
	}
	return false;
}
