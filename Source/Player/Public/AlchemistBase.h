// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AlchemistBase.generated.h"

class UHolderComponent;
class URangeComponent;
class UInputMappingContext;
class UInputAction;
class UInteractableActorFilter;
class UInterfaceActorFilter;
struct FInputActionValue;

UCLASS(Abstract)
class PLAYER_API AAlchemistBase : public ACharacter
{
	GENERATED_BODY()

	AAlchemistBase(const FObjectInitializer& ObjectInitializer);

protected:

	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void NotifyControllerChanged() override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHolderComponent> HolderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	FName HolderParentSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URangeComponent> RangeComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Actor Filter")
	TObjectPtr<UInteractableActorFilter> InteractableFilter;

	UPROPERTY(BlueprintReadOnly, Category = "Actor Filter")
	TObjectPtr<UInterfaceActorFilter> CarriableFilter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MovementMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> PickupOrDropAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ThrowAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	float ThrowForce;

private: // Input
	
	void Input_Move(const FInputActionValue& Value);
	void Input_Dash();
	
	void Input_Interact();
	void Input_PickupOrDrop();
	void Input_Throw();

	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* Interactable);

	UFUNCTION(Server, Reliable)
	void Server_Pickup(AActor* Carriable);

	UFUNCTION(Server, Reliable)
	void Server_Drop();

	UFUNCTION(Server, Reliable)
	void Server_Throw(FVector Direction);
};
