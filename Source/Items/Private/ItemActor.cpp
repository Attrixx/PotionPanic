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
	DOREPLIFETIME(AItemActor, AttachComp)
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

USceneComponent* AItemActor::GetAttachComponent_Implementation()
{
	return StaticMesh->GetAttachParent();
}

bool AItemActor::TryPickup_Implementation(USceneComponent* AttachComponent)
{
	// Already attached to something
	if (StaticMesh->GetAttachParent())
		return false;
	
	return TryAttachTo(AttachComponent);
}

bool AItemActor::TryCatch_Implementation(USceneComponent* AttachComponent)
{
	// Items can only be caught when dropped or thrown
	// Simulating physics is only done in those two cases
	if (!StaticMesh->IsSimulatingPhysics())
		return false;

	// Do not catch something we just dropped/thrown
	if (AttachComponent == AttachComp)
		return false;

	return TryAttachTo(AttachComponent);
}

bool AItemActor::TryTransfer_Implementation(USceneComponent* AttachComponent)
{
	return TryAttachTo(AttachComponent);
}

void AItemActor::Drop_Implementation()
{
	StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	if (TrySnapToGround())
	{
		AttachComp.Reset();
		// Allow other to catch item
		StaticMesh->ClearSkipUpdateOverlaps();
	}
	else
	{
		StaticMesh->SetSimulatePhysics(true);
	}
	
}

void AItemActor::Throw_Implementation(FVector Velocity)
{
	StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	StaticMesh->SetSimulatePhysics(true);
	StaticMesh->SetPhysicsLinearVelocity(Velocity);
}

bool AItemActor::TryAttachTo(USceneComponent* AttachComponent)
{
	StaticMesh->SetSimulatePhysics(false);
	AttachComp = AttachComponent;

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
		return false;
	}

	return true;
}

void AItemActor::Mesh_OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	check(StaticMesh == HitComponent);

	AttachComp.Reset();

	if (Hit.Normal.Dot(FVector::UpVector) >= GroundCollisionThreshold)
	{
		if (TrySnapToGround())
		{
			StaticMesh->SetSimulatePhysics(false);
		}
	}
}

bool AItemActor::TrySnapToGround()
{
	FHitResult HitResult;
	FVector Start = StaticMesh->GetComponentLocation();
	FVector End = Start + FVector::DownVector * SnapToGroundMaxDistance;
	FQuat Rot = StaticMesh->GetComponentQuat();
	auto ProfileName = StaticMesh->GetCollisionProfileName();
	auto CollisionShape = StaticMesh->GetCollisionShape();
	FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
	QueryParams.AddIgnoredActor(this);
	if (GetWorld()->SweepSingleByProfile(HitResult, Start, End, Rot, ProfileName, CollisionShape, QueryParams))
	{
		StaticMesh->SetWorldLocation(HitResult.Location);
		return true;
	}
	return false;
}

void AItemActor::OnRep_AttachComp()
{
	if (AttachComp.IsValid())
	{
		StaticMesh->SetSimulatePhysics(false);
		StaticMesh->AttachToComponent(AttachComp.Get(),
			FAttachmentTransformRules
			{
				EAttachmentRule::SnapToTarget,
				EAttachmentRule::KeepWorld,
				EAttachmentRule::KeepWorld,
				false
			});
	}
	else
	{
		StaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
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
