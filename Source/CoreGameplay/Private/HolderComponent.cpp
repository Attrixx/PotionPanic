// Fill out your copyright notice in the Description page of Project Settings.

#include "HolderComponent.h"
#include "Carriable.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_HolderComponent, Log, All);

UHolderComponent::UHolderComponent()
	: bIsCatchAllowed(true)
	, bIsTransferAllowed(true)
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

bool UHolderComponent::TryPickup(AActor* Actor)
{
	if (!Actor || !Actor->Implements<UCarriable>() || GetHeldActor())
		return false;

	if (ICarriable::Execute_TryPickup(Actor, this))
	{
		HeldActor = Actor;
		return true;
	}
	
	if (auto* OtherHolder = Cast<UHolderComponent>(ICarriable::Execute_GetAttachComponent(Actor)))
	{
		return OtherHolder->TryTransfer(this);	
	}
	
	return false;
}

bool UHolderComponent::TryTransfer(UHolderComponent* Other)
{
	if (bIsTransferAllowed && HeldActor.IsValid() && !Other->HeldActor.IsValid())
	{
		if (ICarriable::Execute_TryTransfer(HeldActor.Get(), Other))
		{
			Other->HeldActor = HeldActor;
			HeldActor.Reset();
			return true;
		}
	}
	return false;
}

AActor* UHolderComponent::Drop()
{
	AActor* Actor = HeldActor.Get();
	HeldActor.Reset();
	
	if (Actor) 
		ICarriable::Execute_Drop(Actor);
	return Actor;
}

AActor* UHolderComponent::Throw(FVector Velocity)
{
	AActor* Actor = HeldActor.Get();
	HeldActor.Reset();

	if (Actor) 
		ICarriable::Execute_Throw(Actor, Velocity);
	return Actor;
}

void UHolderComponent::Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(OverlappedComponent == this);

	if (!OtherActor->Implements<UCarriable>() || !bIsCatchAllowed || HeldActor.IsValid())
		return;

	if (ICarriable::Execute_TryCatch(OtherActor, this))
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
