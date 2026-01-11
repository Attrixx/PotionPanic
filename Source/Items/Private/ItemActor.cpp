// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemRow.h"
#include "SocketableComponent.h"
#include <Components/StaticMeshComponent.h>
#include <Components/AudioComponent.h>
#include <NiagaraComponent.h>

DEFINE_LOG_CATEGORY_STATIC(PP_ItemActor, Log, All);

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SocketableRoot = CreateDefaultSubobject<USocketableComponent>(TEXT("Root"));
	SetRootComponent(SocketableRoot.Get());

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->SetupAttachment(StaticMesh);
	Niagara->SetAutoActivate(true);

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(StaticMesh);
	Audio->bAutoActivate = false;
}

void AItemActor::SetItem(const FItemRow& NewItem)
{
	Item = &NewItem;

	StaticMesh->SetStaticMesh(NewItem.StaticMesh);
	
	Niagara->SetAsset(NewItem.NiagaraSystem);
	Niagara->Activate(true);

	Audio->SetSound(NewItem.Sound);
	Audio->Activate(true);
}
