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

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

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

protected:

	/*
	* Input related methods
	*/
	void Move(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void Carry(const FInputActionValue& Value);
	
};
