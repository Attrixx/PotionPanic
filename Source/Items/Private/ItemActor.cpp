// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemAsset.h"
#include "CarriableComponent.h"
#include <Components/StaticMeshComponent.h>
#include <Components/AudioComponent.h>
#include <NiagaraComponent.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ItemActor, Log, All);

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Carriable = CreateDefaultSubobject<UCarriableComponent>(TEXT("Carriable"));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->SetupAttachment(StaticMesh);
	Niagara->SetAutoActivate(true);

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(StaticMesh);
	Audio->bAutoActivate = false;
}

void AItemActor::OnConstruction(const FTransform& Transform)
{
	if (ItemAsset)
	{
		SetItemAsset(ItemAsset);
	}
}

void AItemActor::SetItemAsset(UItemAsset* NewItemAsset)
{
	if (!NewItemAsset)
	{
		return;
	}

	ItemAsset = NewItemAsset;

	if (StaticMesh)
	{
		StaticMesh->SetStaticMesh(NewItemAsset->StaticMesh);
	}
	
	if (Niagara)
	{
		Niagara->SetAsset(NewItemAsset->NiagaraSystem);
		Niagara->Activate(true);
	}

	if (Audio)
	{
		Audio->SetSound(NewItemAsset->Sound);
		Audio->Activate(true);
	}
}
