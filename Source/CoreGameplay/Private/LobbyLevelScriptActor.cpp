// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyLevelScriptActor.h"
#include "LobbyGameMode.h"
#include "LobbyGameState.h"

#include "Engine/TriggerBox.h"
#include "Components/ShapeComponent.h"
#include "GameFramework/Character.h"

void ALobbyLevelScriptActor::RegisterTriggerBoxes(const TMap<ETriggerBoxType, ATriggerBox*>& TriggerBoxes)
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

void ALobbyLevelScriptActor::OnTriggerBoxBeginOverlap(AActor* TriggerBox, AActor* OtherActor)
{
	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	ALobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALobbyGameMode>() : nullptr;
	if (!IsValid(OverlappingCharacter) || !IsValid(LobbyGameMode)) return;

	if (TriggerBox->ActorHasTag(TEXT("LobbyInterface")))
	{
		LobbyGameMode->OnPlayerEnterArea(OverlappingCharacter, ECameraPosition::LobbyInterface);
	}
	else if (TriggerBox->ActorHasTag(TEXT("Entrance")))
	{
		LobbyGameMode->OnPlayerEnterArea(OverlappingCharacter, ECameraPosition::Entrance);
	}
	else if (TriggerBox->ActorHasTag(TEXT("Interior")))
	{
		LobbyGameMode->OnPlayerEnterArea(OverlappingCharacter, ECameraPosition::Interior);
	}
}

void ALobbyLevelScriptActor::OnTriggerBoxEndOverlap(AActor* TriggerBox, AActor* OtherActor)
{
	ACharacter* OverlappingCharacter = Cast<ACharacter>(OtherActor);
	ALobbyGameMode* LobbyGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ALobbyGameMode>() : nullptr;
	if (!IsValid(OverlappingCharacter) || !IsValid(LobbyGameMode)) return;

	if (TriggerBox->ActorHasTag(TEXT("LobbyInterface")))
	{
		LobbyGameMode->OnPlayerLeaveArea(OverlappingCharacter, ECameraPosition::LobbyInterface);
	}
	else if (TriggerBox->ActorHasTag(TEXT("Entrance")))
	{
		LobbyGameMode->OnPlayerLeaveArea(
			OverlappingCharacter, 
			ECameraPosition::Entrance, 
			IsAnyActorInTriggerBox(AActor::StaticClass(), ETriggerBoxType::Entrance), 
			IsActorInTriggerBox(OverlappingCharacter, ETriggerBoxType::Interior)
		);
	}
	else if (TriggerBox->ActorHasTag(TEXT("Interior")))
	{
		LobbyGameMode->OnPlayerLeaveArea(OverlappingCharacter, ECameraPosition::Interior);
	}
}

bool ALobbyLevelScriptActor::IsAnyActorInTriggerBox(TSubclassOf<AActor> ClassToSearch, ETriggerBoxType TriggerBoxType) const
{
	if (!IsValid(ClassToSearch)) return false;

	ATriggerBox* TriggerBox = RegisteredTriggerBoxes.FindRef(TriggerBoxType);
	UShapeComponent* CollisionComp = TriggerBox->GetCollisionComponent();
	if (!CollisionComp)
	{
		return false;
	}

	TArray<AActor*> OverlappingActors;
	CollisionComp->GetOverlappingActors(OverlappingActors);

	for (const AActor* Actor : OverlappingActors)
	{
		if (Actor->IsA(ClassToSearch))
		{
			return true;
		}
	}
	return false;
}

bool ALobbyLevelScriptActor::IsActorInTriggerBox(AActor* Actor, ETriggerBoxType TriggerBoxType) const
{
	if (!IsValid(Actor)) return false;

	ATriggerBox* TriggerBox = RegisteredTriggerBoxes.FindRef(TriggerBoxType);
	UShapeComponent* CollisionComp = TriggerBox->GetCollisionComponent();
	if (!CollisionComp)
	{
		return false;
	}

	TArray<AActor*> OverlappingActors;
	CollisionComp->GetOverlappingActors(OverlappingActors);

	return OverlappingActors.Contains(Actor);
}
