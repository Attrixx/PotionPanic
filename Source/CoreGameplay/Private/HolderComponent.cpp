// Fill out your copyright notice in the Description page of Project Settings.

#include "HolderComponent.h"
#include "Carriable.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_HolderComponent, Log, All);

UHolderComponent::UHolderComponent()
	: bAllowStealing(false)
	, bShouldSwitchCollisionProfileOnPickup(false)
	, bShouldSwitchCollisionProfileOnRelease(false)
	, bShouldSnapToGroundOnReleaseWithoutVelocity(true)
	, bIsCatchAllowed(true)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	
	SetGenerateOverlapEvents(true);
	// the following delegate is part of our (inherited) members, binding here and never unbinding is fine
	OnComponentBeginOverlap.AddDynamic(this, &UHolderComponent::Sphere_OnBeginOverlap);
}

void UHolderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHolderComponent, Carriable);
}

bool UHolderComponent::TryPickup(UObject* NewCarriable)
{
	if (!NewCarriable || !NewCarriable->Implements<UCarriable>() || Carriable.IsValid())
		return false;

	UPrimitiveComponent* Primitive = ICarriable::Execute_GetPrimitive(NewCarriable);
	if (!Primitive)
	{
		UE_LOGFMT(MS_HolderComponent, Error, "NewCarriable Primitive is null.");
		return false;
	}

	if (USceneComponent* Parent = Primitive->GetAttachParent())
	{
		if (auto* OtherHolder = Cast<UHolderComponent>(Parent))
		{
			if (OtherHolder->bAllowStealing)
			{
				OtherHolder->Release();
			}
			else
			{
				return false;
			}
		}
	}

	// This must be set BEFORE AttachToComponent, because it may trigger
	// Sphere_OnBeginOverlap which will call TryPickup again.
	Carriable = NewCarriable;

	Primitive->SetSimulatePhysics(false);
	bool bAttachSuccess = Primitive->AttachToComponent(this, {LocationRule, RotationRule, ScaleRule, false});

	if (!bAttachSuccess)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "AttachToComponent failed.");
	}

	if (bShouldSwitchCollisionProfileOnPickup)
	{
		FName Profile = ICarriable::Execute_GetCarriedCollisionProfileName(NewCarriable);
		if (Profile.IsNone())
		{
			UE_LOGFMT(MS_HolderComponent, Warning, "Carried Collision Profile Name is None.");
		}
		else
		{
			Primitive->SetCollisionProfileName(Profile);
		}
	}
	
	OnCarriableChanged.Broadcast(this);
	return true;
}

UObject* UHolderComponent::Release(FVector Velocity)
{
	if (!Carriable.IsValid())
		return nullptr;

	UPrimitiveComponent* Primitive = ICarriable::Execute_GetPrimitive(Carriable.Get());
	if (!Primitive)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "Carriable Primitive is null.");
		auto OldCarriable = Carriable.Get();
		Carriable.Reset();
		return OldCarriable;
	}

	Primitive->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	if (bShouldSwitchCollisionProfileOnRelease)
	{
		FName Profile = ICarriable::Execute_GetStandaloneCollisionProfileName(Carriable.Get());
		if (Profile.IsNone())
		{
			UE_LOGFMT(MS_HolderComponent, Warning, "Standalone Collision Profile Name is None.");
		}
		else
		{
			Primitive->SetCollisionProfileName(Profile);
		}
	}

	bool bSnapped = false;
	if (bShouldSnapToGroundOnReleaseWithoutVelocity && Velocity.IsNearlyZero())
	{
		FHitResult HitResult;
		FVector Start = Primitive->GetComponentLocation();
		FVector End = Start + FVector::DownVector * SnapToGroundMaxDistance;
		FQuat Rot = Primitive->GetComponentQuat();
		auto Channel = Primitive->GetCollisionObjectType();
		auto CollisionShape = Primitive->GetCollisionShape();
		FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(Primitive->GetOwner());
		QueryParams.bIgnoreTouches = true; // Touches means Overlaps
		if (GetWorld()->SweepSingleByChannel(HitResult, Start, End, Rot, Channel, CollisionShape, QueryParams))
		{
			Primitive->SetWorldLocation(HitResult.Location);
			bSnapped = true;
		}
	}
	if (!bSnapped)
	{
		Primitive->SetSimulatePhysics(true);
		Primitive->SetPhysicsLinearVelocity(Velocity, false);
	}

	auto OldCarriable = Carriable.Get();
	Carriable.Reset();

	OnCarriableChanged.Broadcast(this);
	return OldCarriable;
}


UObject* UHolderComponent::Eject()
{
	return Release(GetComponentTransform().TransformVectorNoScale(EjectForce));
}


void UHolderComponent::Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(OverlappedComponent == this);

	if (bIsCatchAllowed && TryPickup(OtherActor))
		return;
}

void UHolderComponent::OnRep_Carriable()
{
	if (Carriable.IsValid())
		if (TryPickup(Carriable.Get()))
			return; // TryPickup already broadcasts

	OnCarriableChanged.Broadcast(this);
}
