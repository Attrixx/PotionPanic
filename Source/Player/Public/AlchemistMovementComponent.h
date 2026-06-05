// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AlchemistMovementComponent.generated.h"

struct FSavedMove_Alchemist : FSavedMove_Character
{
	typedef FSavedMove_Character Super;

	FSavedMove_Alchemist();

	uint8 bWantsToDash : 1;
	float LastDashTime;

	bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	void Clear() override;
	uint8 GetCompressedFlags() const override;
	void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	void PrepMoveFor(ACharacter* C) override;
};

struct FNetworkPredictionData_Client_Alchemist : FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_Alchemist(const UCharacterMovementComponent& ClientMovement);
	FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class PLAYER_API UAlchemistMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

protected:

	UAlchemistMovementComponent();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	void Dash();

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	void SetDashEnabled(bool bEnabled) { bIsDashEnabled = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	bool IsDashEnabled() const { return bIsDashEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	float GetDashRemainingCooldown() const;

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	bool CanDash() const;

protected: // CMC overrides

	void UpdateFromCompressedFlags(uint8 Flags) override;
	FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	void PerformMovement(float DeltaTime) override;

private:

	float GetServerTime() const;
	void PerformDash();

protected:

	UPROPERTY(Replicated, EditDefaultsOnly, Category = "Dash")
	bool bIsDashEnabled = true;

	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0.0", Units = "cm/s"))
	float DashImpulseStrength = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dash", meta = (ClampMin = "0.0", Units = "s"))
	float DashCooldownDuration = 1.f;

private:

	friend FSavedMove_Alchemist;

	uint8 bWantsToDash : 1;

	UPROPERTY(Replicated)
	float LastDashTime;
};
