// Fill out your copyright notice in the Description page of Project Settings.

#include "Carriable.h"

UPrimitiveComponent* ICarriable::GetPrimitive_Implementation() const
{
	return nullptr;
}

FName ICarriable::GetStandaloneCollisionProfileName_Implementation() const
{
	return NAME_None;
}

FName ICarriable::GetCarriedCollisionProfileName_Implementation() const
{
	return NAME_None;
}

void ICarriable::OnPickup_Implementation(USceneComponent* AttachComponent)
{
}

void ICarriable::OnDrop_Implementation()
{
}

void ICarriable::OnThrow_Implementation(FVector Velocity)
{
}
