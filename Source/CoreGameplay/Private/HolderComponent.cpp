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

bool UHolderComponent::TryPickup(AActor* Actor)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "TryPickup must execute on authority. Call ignored.");
		return false;
	}

	if (!Actor || !Actor->Implements<UCarriable>() || HeldActor.IsValid())
		return false;
	
	ICarriable::Execute_Pickup(Actor, this);
	HeldActor = Actor;
	return true;
}

AActor* UHolderComponent::Drop()
{
	AActor* Actor = HeldActor.Get();
	HeldActor.Reset();
	
	ICarriable::Execute_Drop(Actor);
	return Actor;
}

AActor* UHolderComponent::Throw(FVector Velocity)
{
	AActor* Actor = HeldActor.Get();
	HeldActor.Reset();
	
	ICarriable::Execute_Throw(Actor, Velocity);
	return Actor;
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
