// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemAsset.h"
#include "CarriableComponent.h"
#include <Components/StaticMeshComponent.h>
#include <Components/AudioComponent.h>
#include <Kismet/GameplayStatics.h>
#include <NiagaraComponent.h>
#include <NiagaraFunctionLibrary.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ItemActor, Log, All);

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Carriable = CreateDefaultSubobject<UCarriableComponent>(TEXT("Carriable"));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetNotifyRigidBodyCollision(true);

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->SetupAttachment(StaticMesh);
	Niagara->SetAutoActivate(true);

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(StaticMesh);
	Audio->bAutoActivate = false;
}

void AItemActor::OnConstruction(const FTransform &Transform)
{
	if (ItemAsset)
	{
		SetItemAsset(ItemAsset);
	}
}

void AItemActor::NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	if (IsBreakable() && NormalImpulse.Size() >= GetBreakImpulseThreshold())
	{
		DestroyItem(true);
	}
}

void AItemActor::SetItemAsset(UItemAsset &NewItemAsset)
{
	if (!NewItemAsset)
	{
		return;
	}

	ItemAsset = NewItemAsset;
	ApplyVisualsFromAsset(NewItemAsset);

	if (UWorld *World = GetWorld(); World && World->IsGameWorld())
	{
		PlaySpawnFeedback();
	}

	if (Carriable)
	{
		Carriable->SetItemId(NewItemAsset->GetPrimaryAssetId());
	}
}

void AItemActor::DestroyItem(bool bPlayFeedback)
{
	if (bPlayFeedback && GetWorld() && GetWorld()->IsGameWorld())
	{
		PlayDestroyFeedback();
	}

	Destroy();
}

bool AItemActor::IsBreakable() const
{
	return ItemAsset && ItemAsset->bIsDestructible;
}

float AItemActor::GetBreakImpulseThreshold() const
{
	return ItemAsset ? ItemAsset->BreakImpulseThreshold : 0.0f;
}

TArray<FName> AItemActor::GetTransformationFlags() const
{
	return ItemAsset ? ItemAsset->TransformationFlags : TArray<FName>{};
}

void AItemActor::ApplyVisualsFromAsset(UItemAsset *NewItemAsset)
{
	if (StaticMesh)
	{
		StaticMesh->SetStaticMesh(NewItemAsset->StaticMesh);

		if (NewItemAsset->MaterialOverride)
		{
			StaticMesh->SetMaterial(0, NewItemAsset->MaterialOverride);
		}
	}

	if (Niagara)
	{
		Niagara->SetAsset(NewItemAsset->NiagaraSystem);
		Niagara->SetAutoActivate(NewItemAsset->NiagaraSystem != nullptr);
		if (NewItemAsset->NiagaraSystem)
		{
			Niagara->Activate(true);
		}
		else
		{
			Niagara->Deactivate();
		}
	}

	if (Audio)
	{
		Audio->SetSound(NewItemAsset->Sound);
		if (NewItemAsset->Sound)
		{
			Audio->Play();
		}
		else
		{
			Audio->Stop();
		}
	}
}

void AItemActor::PlaySpawnFeedback() const
{
	if (ItemAsset == nullptr)
	{
		return;
	}

	if (ItemAsset->SpawnEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ItemAsset->SpawnEffect,
			GetActorLocation(),
			GetActorRotation());
	}

	if (ItemAsset->SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ItemAsset->SpawnSound, GetActorLocation());
	}
}

void AItemActor::PlayDestroyFeedback() const
{
	if (ItemAsset == nullptr)
	{
		return;
	}

	if (ItemAsset->DestroyEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ItemAsset->DestroyEffect,
			GetActorLocation(),
			GetActorRotation());
	}

	if (ItemAsset->DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ItemAsset->DestroySound, GetActorLocation());
	}
}
