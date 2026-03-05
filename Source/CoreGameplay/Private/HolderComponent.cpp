// Fill out your copyright notice in the Description page of Project Settings.

#include "HolderComponent.h"
#include "Carriable.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_HolderComponent, Log, All);

UHolderComponent::UHolderComponent()
	: bIsCatchAllowed(true)
	, bIsTransferAllowedOnCatchFailure(false)
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

bool UHolderComponent::TryPickup(AActor* Actor, bool bIsTransferAllowedOnFailure)
{
	if (!Actor || !Actor->Implements<UCarriable>() || GetHeldActor())
		return false;

	// First try to pickup
	if (ICarriable::Execute_TryPickup(Actor, this))
	{
		HeldActor = Actor;
		return true;
	}

	// If pickup failed, maybe we can transfer?
	if (bIsTransferAllowedOnFailure)
		if (auto* OtherHolder = Cast<UHolderComponent>(ICarriable::Execute_GetAttachComponent(Actor)))
		{
			return OtherHolder->TryTransfer(this);
		}

	return false;
}

bool UHolderComponent::TryTransfer(UHolderComponent* Dest)
{
	// Is transfer valid?
	if (bIsTransferAllowed && HeldActor.IsValid() && !Dest->HeldActor.IsValid())
	{
		// Is transfer successful?
		if (ICarriable::Execute_TryTransfer(HeldActor.Get(), Dest))
		{
			Dest->HeldActor = HeldActor;
			HeldActor.Reset();
			return true;
		}
	}
	return false;
}

AActor* UHolderComponent::Drop()
{
	if (AActor* Actor = HeldActor.Get())
	{
		HeldActor.Reset();
		ICarriable::Execute_Drop(Actor);
		return Actor;
	}
	return nullptr;
}

AActor* UHolderComponent::Throw(FVector Velocity)
{
	if (AActor* Actor = HeldActor.Get())
	{
		HeldActor.Reset();
		ICarriable::Execute_Throw(Actor, Velocity);
	}
	return nullptr;
}

void UHolderComponent::Sphere_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	check(OverlappedComponent == this);

	// Is catch valid?
	if (bIsCatchAllowed && OtherActor->Implements<UCarriable>() && !HeldActor.IsValid())
	{
		// Is catch successful?
		if (ICarriable::Execute_TryCatch(OtherActor, this))
		{
			HeldActor = OtherActor;
		}

		// If pickup failed, maybe we can transfer?
		else if (bIsTransferAllowedOnCatchFailure)
			if (auto* OtherHolder = Cast<UHolderComponent>(ICarriable::Execute_GetAttachComponent(OtherActor)))
			{
				OtherHolder->TryTransfer(this);
			}
	}
}

void UHolderComponent::OnRep_HeldActor(TWeakObjectPtr<AActor> OldActor)
{
	if (OldActor.Get() != HeldActor.Get())
	{
		OnCarriableChanged.Broadcast(this, OldActor.Get(), HeldActor.Get());
	}

	// NOTE: The carriable is responsible for replicating the attachment
}
