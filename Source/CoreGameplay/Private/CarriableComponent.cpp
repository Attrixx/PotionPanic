// Fill out your copyright notice in the Description page of Project Settings.

#include "CarriableComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_CarriableComponent, Log, All);

UCarriableComponent::UCarriableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UCarriableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCarriableComponent, Holder);
}

void UCarriableComponent::SetHolder(UHolderComponent* NewHolder)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOGFMT(MS_CarriableComponent, Warning, "SetHolder must execute on authority. Call ignored.");
		return;
	}
	
	UHolderComponent* OldHolder = std::exchange(Holder, NewHolder);
	OnHolderChanged(OldHolder, NewHolder);
}

void UCarriableComponent::OnRep_Holder(UHolderComponent* OldHolder)
{
	OnHolderChanged(OldHolder, Holder);
}

void UCarriableComponent::OnHolderChanged_Implementation(UHolderComponent* OldHolder, UHolderComponent* NewHolder)
{
	// Nothing to do by default, can be overriden in bp or cpp.
}
