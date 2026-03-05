// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemAsset.h"
#include <Net/UnrealNetwork.h>
#include <Components/StaticMeshComponent.h>
#include <Components/AudioComponent.h>
#include <NiagaraComponent.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ItemActor, Log, All);

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

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
		SetItemAsset(*ItemAsset);
	}
}

void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->OnComponentHit.AddDynamic(this, &AItemActor::Mesh_OnHit);
}

void AItemActor::SetItemAsset(UItemAsset& NewItemAsset)
{
	ItemAsset = &NewItemAsset;
	ApplyItemAsset();
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

	if (Hit.Normal.Dot(FVector::UpVector) >= GroundCollisionThreshold)
	{
		StaticMesh->SetSimulatePhysics(false);
	}
}

void AItemActor::ApplyItemAsset()
{
	if (!IsValid(ItemAsset))
		return;

	StaticMesh->SetStaticMesh(ItemAsset->StaticMesh);

	Niagara->SetAsset(ItemAsset->NiagaraSystem);
	Niagara->Activate(true);

	Audio->SetSound(ItemAsset->Sound);
	Audio->Activate(true);
}
