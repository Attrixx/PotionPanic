// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemAsset.h"
#include "ItemTags.h"
#include <Net/UnrealNetwork.h>
#include <Components/StaticMeshComponent.h>
#include <Components/AudioComponent.h>
#include <NiagaraComponent.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ItemActor, Log, All);

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetNotifyRigidBodyCollision(true);
	StaticMesh->BodyInstance.bLockXRotation = true;
	StaticMesh->BodyInstance.bLockYRotation = true;

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->SetupAttachment(StaticMesh);
	Niagara->SetAutoActivate(true);

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(StaticMesh);
	Audio->bAutoActivate = false;
}

void AItemActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AItemActor, ItemAsset)
}

void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ItemAsset)
	{
		ApplyItemAsset();
	}
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->OnComponentHit.AddDynamic(this, &AItemActor::Mesh_OnHit);
}

void AItemActor::SetItemAsset(UItemAsset* NewItemAsset)
{
	if (HasAuthority())
	{
		ItemAsset = NewItemAsset;
		OnRep_ItemAsset();
	}
}

void AItemActor::SetItemTags(const FGameplayTagContainer& NewItemTags)
{
	check(NewItemTags == NewItemTags.Filter(FGameplayTagContainer(GameTags::Item)));
	ItemTags = NewItemTags;
}

void AItemActor::AppendItemTags(const FGameplayTagContainer& NewItemTags)
{
	check(NewItemTags == NewItemTags.Filter(FGameplayTagContainer(GameTags::Item)));
	ItemTags.AppendTags(NewItemTags);
}

void AItemActor::RemoveItemTag(const FGameplayTagContainer& ItemTagsToRemove)
{
	check(ItemTagsToRemove == ItemTagsToRemove.Filter(FGameplayTagContainer(GameTags::Item)));
	ItemTags.RemoveTags(ItemTagsToRemove);
}

UPrimitiveComponent* AItemActor::GetPrimitive_Implementation() const
{
	return StaticMesh;
}

FName AItemActor::GetStandaloneCollisionProfileName_Implementation() const
{
	return StandaloneCollisionProfileName;
}

FName AItemActor::GetCarriedCollisionProfileName_Implementation() const
{
	return CarriedCollisionProfileName;
}

void AItemActor::Mesh_OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	check(StaticMesh == HitComponent);
	
	if (ItemTags.HasTag(GameTags::Item_Breakable))
	{
		// TODO: Implement breakable items
		UE_LOGFMT(MS_ItemActor, Warning, "Breakable items are not implemented.");
	}

	if (Hit.ImpactNormal.Dot(FVector::UpVector) >= GroundCollisionThreshold)
	{
		StaticMesh->SetSimulatePhysics(false);
	}
}

void AItemActor::OnRep_ItemAsset()
{
	if (IsValid(ItemAsset))
	{
		ApplyItemAsset();
	}
}

void AItemActor::ApplyItemAsset()
{
	check(IsValid(ItemAsset));

	ItemTags = ItemAsset->ItemTags;

	StaticMesh->SetStaticMesh(ItemAsset->StaticMesh);

	Niagara->SetAsset(ItemAsset->NiagaraSystem);
	Niagara->Activate(true);

	Audio->SetSound(ItemAsset->Sound);
	Audio->Activate(true);
}
