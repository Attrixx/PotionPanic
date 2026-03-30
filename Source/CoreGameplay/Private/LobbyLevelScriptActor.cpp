// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyLevelScriptActor.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"
#include "LobbyCharacter.h"

#include "Engine/TriggerBox.h"
#include "Components/ShapeComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

void ALobbyLevelScriptActor::BeginPlay()
{
	Super::BeginPlay();
	ToggleShowPlayerPreviews(false);
}

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

void ALobbyLevelScriptActor::ToggleShowPlayerPreviews(bool bShow)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ALobbyPlayerController* PlayerController = Cast<ALobbyPlayerController>(Iterator->Get());
		if (!IsValid(PlayerController)) continue;

		if (ACharacter* PlayerCharacter = PlayerController->GetCharacter())
		{
			PlayerCharacter->SetActorHiddenInGame(bShow);
		}
		if (ACharacter* PreviewCharacter = PlayerController->GetPreviewActor())
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
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
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
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
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
