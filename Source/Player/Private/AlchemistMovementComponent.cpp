// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistMovementComponent.h"
#include <GameFramework/Character.h>

UAlchemistMovementComponent::UAlchemistMovementComponent()
	: DashImpulseStrength(1000.f)
	, DashCooldownDuration(1.f)
	, LastDashTime(-DashCooldownDuration)
{
}

void UAlchemistMovementComponent::Dash()
{
	if (GetDashCooldown() > 0.f)
		return;

	PerformDash();

	if (CharacterOwner && !CharacterOwner->HasAuthority())
	{
		Server_Dash();
	}
}

float UAlchemistMovementComponent::GetDashCooldown() const
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float TimeRemaining = LastDashTime + DashCooldownDuration - CurrentTime;
	return FMath::Max(0.f, TimeRemaining);
}

void UAlchemistMovementComponent::Server_Dash_Implementation()
{
	if (GetDashCooldown() <= 0.f)
	{
		PerformDash();
	}
}

void UAlchemistMovementComponent::PerformDash()
{
	if (!CharacterOwner)
		return;

	LastDashTime = GetWorld()->GetTimeSeconds();

	FVector DashDirection = Velocity.IsNearlyZero()
		? CharacterOwner->GetActorForwardVector()
		: Velocity.GetSafeNormal2D();

	FVector DashVelocity = DashDirection * DashImpulseStrength;
	CharacterOwner->LaunchCharacter(DashVelocity, true, false);
}
