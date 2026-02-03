// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AlchemistMovementComponent.generated.h"


UCLASS()
class PLAYER_API UAlchemistMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	UAlchemistMovementComponent();
	
public:
	
	UFUNCTION(BlueprintCallable)
	void Dash();
	
	UFUNCTION(BlueprintCallable)
	float GetDashCooldown() const;
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashImpulseStrength;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashCooldownDuration;
	
	float LastDashTime;
	
private:
	
	UFUNCTION(Server, Reliable)
	void Server_Dash();
	
	void PerformDash();
};
