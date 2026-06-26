// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemistMovementComponent.h"
#include <GameFramework/Character.h>
#include <GameFramework/GameStateBase.h>
#include <Net/UnrealNetwork.h>

FSavedMove_Alchemist::FSavedMove_Alchemist()
	: bWantsToDash(false)
	, LastDashTime(0.f)
{
}

bool FSavedMove_Alchemist::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	auto* NewAlchemistMove = static_cast<const FSavedMove_Alchemist*>(NewMove.Get());
	if (bWantsToDash != NewAlchemistMove->bWantsToDash)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_Alchemist::Clear()
{
	Super::Clear();
	bWantsToDash = false;
}

uint8 FSavedMove_Alchemist::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bWantsToDash)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

void FSavedMove_Alchemist::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	if (auto* MC = Cast<UAlchemistMovementComponent>(Character->GetCharacterMovement()))
	{
		bWantsToDash = MC->bWantsToDash;
		LastDashTime = MC->LastDashTime;
	}
}

void FSavedMove_Alchemist::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	if (auto* MC = Cast<UAlchemistMovementComponent>(Character->GetCharacterMovement()))
	{
		MC->bWantsToDash = bWantsToDash;
		MC->LastDashTime = LastDashTime;
	}
}

FNetworkPredictionData_Client_Alchemist::FNetworkPredictionData_Client_Alchemist(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_Alchemist::AllocateNewMove()
{
	return MakeShared<FSavedMove_Alchemist>();
}

UAlchemistMovementComponent::UAlchemistMovementComponent()
	: bWantsToDash(false)
	, LastDashTime(TNumericLimits<float>::Lowest())
{
}

void UAlchemistMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bIsDashEnabled);
}

void UAlchemistMovementComponent::Dash()
{
	if (CanDash())
	{
		bWantsToDash = true;
	}
}

float UAlchemistMovementComponent::GetDashRemainingCooldown() const
{
	return FMath::Max(0.f, LastDashTime + DashCooldownDuration - GetServerTime());
}

bool UAlchemistMovementComponent::CanDash() const
{
	return IsDashEnabled()
		&& MovementMode != MOVE_None
		&& GetDashRemainingCooldown() <= 0.0f;
}

void UAlchemistMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToDash = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UAlchemistMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UAlchemistMovementComponent* MutableThis = const_cast<UAlchemistMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Alchemist(*this);
	}

	return ClientPredictionData;
}

void UAlchemistMovementComponent::PerformMovement(float DeltaTime)
{
	if (bWantsToDash)
	{
		PerformDash();
		bWantsToDash = false;
	}

	Super::PerformMovement(DeltaTime);
}

float UAlchemistMovementComponent::GetServerTime() const
{
	if (const AGameStateBase* GS = GetWorld()->GetGameState())
		return GS->GetServerWorldTimeSeconds();

	return GetWorld()->GetTimeSeconds();
}

void UAlchemistMovementComponent::PerformDash()
{
	LastDashTime = GetServerTime();

	FVector DashDirection = CharacterOwner && Acceleration.IsNearlyZero()
		? CharacterOwner->GetActorForwardVector()
		: Acceleration;

	DashDirection.Z = 0.0f;
	DashDirection = DashDirection.GetSafeNormal();

	Velocity.X = DashDirection.X * DashImpulseStrength;
	Velocity.Y = DashDirection.Y * DashImpulseStrength;
	SetMovementMode(MOVE_Falling);
}
