// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PotionPanicPlayerController.generated.h"

class APotionPanicCharacter;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class POTIONPANIC_API APotionPanicPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	APotionPanicPlayerController();

protected:

	void BeginPlay() override;
	void SetupInputComponent() override;

private:

	bool bCanDash = true;
	float DashZForce = 50.f;

protected:

	APotionPanicCharacter* PotionPanicCharacter;

	/*
	*	INPUT MAPPING
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CarryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float RotationSpeedScale = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float DashCooldown = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float DashStrength = 1000.f;

protected:

	/*
	* Input related methods
	*/
	void Move(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void Carry(const FInputActionValue& Value);
	void Dash(const FInputActionValue& Value);

public:

	bool IsDashAvailable() const { return bCanDash; }
	
};
