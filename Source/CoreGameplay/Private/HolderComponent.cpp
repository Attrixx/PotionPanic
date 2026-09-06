// Fill out your copyright notice in the Description page of Project Settings.

#include "HolderComponent.h"
#include "Carriable.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(MS_HolderComponent, Log, All);

UHolderComponent::UHolderComponent()
	: bAllowStealing(false)
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

	// A replicated USceneComponent also replicates its own scene graph, which here is a second
	// channel fighting the attachment derived from Carriable. Every machine places the holder the
	// same way from construction, so none of it is worth sending.
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, AttachParent);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, AttachChildren);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, AttachSocketName);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, bShouldBeAttached);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeLocation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeRotation);
	DISABLE_REPLICATED_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D);

	DOREPLIFETIME(UHolderComponent, Carriable);
}

bool UHolderComponent::HasHolderAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}

bool UHolderComponent::TryPickup(UObject* NewCarriable)
{
	if (!NewCarriable || !NewCarriable->Implements<UCarriable>() || GetCarriable())
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
			return OtherHolder->bAllowStealing && OtherHolder->TransferTo(this);
		}
	}

	// This must be set BEFORE attaching, because attaching may trigger
	// Sphere_OnBeginOverlap which will call TryPickup again.
	SetCarriableInternal(NewCarriable);

	AttachCarriable(NewCarriable);
	LocallyAppliedCarriable = NewCarriable;

	OnCarriableChanged.Broadcast(this);
	return true;
}

UObject* UHolderComponent::Release(FVector Velocity)
{
	UObject* OldCarriable = GetCarriable();
	if (!OldCarriable)
		return nullptr;

	UPrimitiveComponent* Primitive = ICarriable::Execute_GetPrimitive(OldCarriable);
	if (!Primitive)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "Carriable Primitive is null.");
		SetCarriableInternal(nullptr);
		return OldCarriable;
	}

	DetachCarriable(OldCarriable);
	LocallyAppliedCarriable.Reset();

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

	SetCarriableInternal(nullptr);

	// A velocity is what separates a throw from a drop, and the only distinction the Carriable
	// itself cares about. Ejecting counts as a throw: it is one, just not a player's.
	if (!Velocity.IsNearlyZero())
	{
		ICarriable::Execute_OnThrow(OldCarriable, Velocity);
	}

	OnCarriableChanged.Broadcast(this);
	return OldCarriable;
}


UObject* UHolderComponent::Eject()
{
	return Release(GetComponentTransform().TransformVectorNoScale(EjectForce));
}

UObject* UHolderComponent::Detach()
{
	UObject* OldCarriable = GetCarriable();
	if (!OldCarriable)
		return nullptr;

	// Restores the standalone collision profile, but leaves physics simulation off: the caller
	// either re-attaches the Carriable right away, or is not in a world that could simulate it.
	DetachCarriable(OldCarriable);
	LocallyAppliedCarriable.Reset();
	SetCarriableInternal(nullptr);

	OnCarriableChanged.Broadcast(this);
	return OldCarriable;
}

bool UHolderComponent::TransferTo(UHolderComponent* Target)
{
	if (!Target || Target == this || !GetCarriable() || Target->GetCarriable())
		return false;

	UObject* Moving = GetCarriable();
	UPrimitiveComponent* Primitive = ICarriable::Execute_GetPrimitive(Moving);
	if (!Primitive)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "Carriable Primitive is null.");
		return false;
	}

	DetachCarriable(Moving);
	LocallyAppliedCarriable.Reset();
	SetCarriableInternal(nullptr);

	if (Target->TryPickup(Moving))
	{
		OnCarriableChanged.Broadcast(this);
		return true;
	}

	// Put it back. A failed transfer leaves both holders exactly as they were.
	TryPickup(Moving);
	return false;
}


void UHolderComponent::Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(OverlappedComponent == this);

	if (!HasHolderAuthority())
		return;

	if (bIsCatchAllowed && TryPickup(OtherActor))
		return;
}

UHolderComponent* UHolderComponent::FindCarryingHolder(const UPrimitiveComponent* Primitive)
{
	for (USceneComponent* Parent = Primitive ? Primitive->GetAttachParent() : nullptr; Parent; Parent = Parent->GetAttachParent())
	{
		if (UHolderComponent* Holder = Cast<UHolderComponent>(Parent))
		{
			return Holder;
		}
	}

	return nullptr;
}

void UHolderComponent::RefreshCarriedState(UObject* InCarriable)
{
	if (!InCarriable)
		return;

	UPrimitiveComponent* Primitive = ICarriable::Execute_GetPrimitive(InCarriable);
	if (!Primitive)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "RefreshCarriedState: Primitive is null.");
		return;
	}

	// Attached or free, and nothing else decides it.
	const UHolderComponent* Carrier = FindCarryingHolder(Primitive);

	const FName Profile = Carrier
		? ICarriable::Execute_GetCarriedCollisionProfileName(InCarriable)
		: ICarriable::Execute_GetStandaloneCollisionProfileName(InCarriable);

	if (Profile.IsNone())
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "{0} collision profile name is None on '{1}'.",
			Carrier ? TEXT("Carried") : TEXT("Standalone"), GetNameSafe(InCarriable));
	}
	else
	{
		Primitive->SetCollisionProfileName(Profile);
	}

	// Only Release turns physics back on, once it knows the item is not going into another holder.
	if (Carrier)
	{
		Primitive->SetSimulatePhysics(false);
	}
}

void UHolderComponent::AttachCarriable(UObject* InCarriable)
{
	UPrimitiveComponent* Primitive = InCarriable ? ICarriable::Execute_GetPrimitive(InCarriable) : nullptr;
	if (!Primitive)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "AttachCarriable: Primitive is null.");
		return;
	}

	// Before the attachment: a simulating body keeps driving its own transform and ignores its parent.
	Primitive->SetSimulatePhysics(false);

	if (!Primitive->AttachToComponent(this, {LocationRule, RotationRule, ScaleRule, false}))
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "AttachToComponent failed.");
	}

	RefreshCarriedState(InCarriable);
}

void UHolderComponent::DetachCarriable(UObject* InCarriable)
{
	UPrimitiveComponent* Primitive = InCarriable ? ICarriable::Execute_GetPrimitive(InCarriable) : nullptr;
	if (!Primitive)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "DetachCarriable: Primitive is null.");
		return;
	}

	// Something else holds it already: detaching would undo the receiving holder's attach.
	if (Primitive->GetAttachParent() != this)
	{
		return;
	}

	Primitive->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	RefreshCarriedState(InCarriable);
}

void UHolderComponent::SetCarriableInternal(UObject* InCarriable)
{
	if (HasHolderAuthority())
	{
		Carriable = InCarriable;
		return;
	}

	// Off the authority the answer is a round trip away: act on the request now, get corrected later.
	PredictedCarriable = InCarriable;
	bHasPrediction = true;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(PredictionTimer, this,
			&UHolderComponent::PredictionTimedOut, PredictionTimeoutSeconds, false);
	}
}

void UHolderComponent::ClearPrediction()
{
	bHasPrediction = false;
	PredictedCarriable.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PredictionTimer);
	}
}

void UHolderComponent::PredictionTimedOut()
{
	ClearPrediction();
	ApplyCarriable();

	OnCarriableChanged.Broadcast(this);
}

void UHolderComponent::ApplyCarriable()
{
	UObject* NewCarriable = GetCarriable();
	UObject* PrevCarriable = LocallyAppliedCarriable.Get();

	if (NewCarriable != PrevCarriable)
	{
		// The attachment is derived here rather than awaited from AttachmentReplication: that second
		// channel can contradict Carriable, and nothing would come back to settle the disagreement.
		if (PrevCarriable)
			DetachCarriable(PrevCarriable);

		if (NewCarriable)
			AttachCarriable(NewCarriable);

		LocallyAppliedCarriable = NewCarriable;
	}
}

void UHolderComponent::OnRep_Carriable()
{
	// The server has answered: right or wrong, the guess has no say any more.
	ClearPrediction();
	ApplyCarriable();

	OnCarriableChanged.Broadcast(this);
}
