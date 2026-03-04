// Fill out your copyright notice in the Description page of Project Settings.

#include "HolderComponent.h"
#include "Carriable.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_HolderComponent, Log, All);

UHolderComponent::UHolderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHolderComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHolderComponent, HeldActor);
}

void UHolderComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UHolderComponent::Sphere_OnBeginOverlap);
}

AActor* UHolderComponent::GetHeldActor() const
{
	if (HeldActor.IsValid() && HeldActor->GetRootComponent()->GetAttachParent() != this)
		HeldActor.Reset();
	return HeldActor.Get();
}

bool UHolderComponent::TryPickup(AActor* Actor)
{
	if (!Actor || !Actor->Implements<UCarriable>() || GetHeldActor())
		return false;

	if (!ICarriable::Execute_TryPickup(Actor, this))
		return false;

	HeldActor = Actor;
	return true;
}

AActor* UHolderComponent::Drop()
{
	AActor* Actor = GetHeldActor();
	HeldActor.Reset();

	ICarriable::Execute_Drop(Actor);
	return Actor;
}

AActor* UHolderComponent::Throw(FVector Velocity)
{
	AActor* Actor = GetHeldActor();
	HeldActor.Reset();

	ICarriable::Execute_Throw(Actor, Velocity);
	return Actor;
}

void UHolderComponent::Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(OverlappedComponent == this);
	
	if (bIsCatchAllowed && !GetHeldActor() && ICarriable::Execute_TryCatch(OtherActor, this))
	{
		HeldActor = OtherActor;
	}
}

void UHolderComponent::OnRep_HeldCarriable(TWeakObjectPtr<AActor> OldCarriable)
{
	if (OldCarriable.Get() != HeldActor.Get())
	{
		OnCarriableChanged(OldCarriable.Get(), HeldActor.Get());
	}
}

void UHolderComponent::OnCarriableChanged_Implementation(AActor* OldCarriable, AActor* NewCarriable)
{
	// Nothing to do by default
}
