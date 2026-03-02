// Fill out your copyright notice in the Description page of Project Settings.

#include "HolderComponent.h"
#include "CarriableComponent.h"
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
	DOREPLIFETIME(UHolderComponent, Carriable);
}

UCarriableComponent* UHolderComponent::Replace(UCarriableComponent* NewCarriable)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "Replace must execute on authority. Call ignored.");
		return Carriable;
	}

	if (Carriable == NewCarriable)
	{
		return NewCarriable;
	}

	if (Carriable)
	{
		Carriable->SetHolder(nullptr);
	}

	if (NewCarriable)
	{
		NewCarriable->SetHolder(this);
	}

	UCarriableComponent* OldCarriable = std::exchange(Carriable, NewCarriable);
	OnCarriableChanged(OldCarriable, NewCarriable);
	return OldCarriable;
}

void UHolderComponent::Throw()
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOGFMT(MS_HolderComponent, Warning, "Throw must execute on authority. Call ignored.");
		return;
	}

}

void UHolderComponent::OnRep_Carriable(UCarriableComponent* OldCarriable)
{
	OnCarriableChanged(OldCarriable, Carriable);
}

void UHolderComponent::OnCarriableChanged_Implementation(UCarriableComponent* OldCarriable, UCarriableComponent* NewCarriable)
{
	if (OldCarriable)
	{
		if (AActor* OldActor = OldCarriable->GetOwner())
		{
			OldActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		}
		else
		{
			UE_LOGFMT(MS_HolderComponent, Warning, "Could not detach Old Carriable from null owner.");
		}
	}

	if (NewCarriable)
	{
		if (AActor* NewActor = NewCarriable->GetOwner())
		{
			NewActor->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		}
		else
		{
			UE_LOGFMT(MS_HolderComponent, Warning, "Could not attach New Carriable to null owner.");
		}
	}
}
