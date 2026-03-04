// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemAsset.h"
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

	if (GetLocalRole() == ROLE_Authority)
	{
		StaticMesh->OnComponentHit.AddDynamic(this, &AItemActor::Mesh_OnHit);
	}
}

void AItemActor::SetItemAsset(UItemAsset& NewItemAsset)
{
	ItemAsset = &NewItemAsset;

	StaticMesh->SetStaticMesh(NewItemAsset.StaticMesh);

	Niagara->SetAsset(NewItemAsset.NiagaraSystem);
	Niagara->Activate(true);

	Audio->SetSound(NewItemAsset.Sound);
	Audio->Activate(true);
}

void AItemActor::Pickup_Implementation(USceneComponent* AttachComponent)
{
	bool bSuccess = StaticMesh->AttachToComponent(AttachComponent,
		FAttachmentTransformRules
		{
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld,
			EAttachmentRule::KeepWorld,
			false
		});

	if (!bSuccess)
	{
		UE_LOGFMT(MS_ItemActor, Error, "Item failed to attach in Pickup.");
	}

	StaticMesh->SetSimulatePhysics(false);
}

void AItemActor::Drop_Implementation()
{
	StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	StaticMesh->SetSimulatePhysics(true);
}

void AItemActor::Throw_Implementation(FVector Velocity)
{
	StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->SetPhysicsLinearVelocity(Velocity);
}

void AItemActor::Mesh_OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	check(StaticMesh == HitComponent);
	check(GetLocalRole() == ROLE_Authority);
	
	if (Hit.Normal.Dot(FVector::UpVector) >= GroundCollisionThreshold)
	{
		StaticMesh->SetSimulatePhysics(false);
		SnapToGround();
	}
}

void AItemActor::SnapToGround()
{
	FHitResult HitResult;
	FVector Start = StaticMesh->GetComponentLocation();
	FVector End = Start + FVector::DownVector * SnapToGroundMaxDistance;
	FQuat Rot = StaticMesh->GetComponentQuat();
	auto ProfileName = StaticMesh->GetCollisionProfileName();
	auto CollisionShape = StaticMesh->GetCollisionShape();
	FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
	QueryParams.AddIgnoredActor(GetOwner());
	if (!GetWorld()->SweepSingleByProfile(HitResult, Start, End, Rot, ProfileName, CollisionShape, QueryParams))
	{
		UE_LOGFMT(MS_ItemActor, Error, "Impossible to snap: ground not found.");
		return;
	}

	StaticMesh->SetWorldLocation(HitResult.Location);
}
