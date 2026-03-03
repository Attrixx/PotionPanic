// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AlchemistBase.generated.h"

class UHolderComponent;
class UCapsuleComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

class IInteractable;
class UCarriableComponent;

UCLASS(Abstract)
class PLAYER_API AAlchemistBase : public ACharacter
{
	GENERATED_BODY()

	AAlchemistBase(const FObjectInitializer& ObjectInitializer);

protected:

	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void Tick(float DeltaSeconds) override;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHolderComponent> HolderComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	FName HolderParentSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> CapsuleOverlapComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

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

private:

	void UpdateBestComponents();

	UFUNCTION()
	void Capsule_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                            bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Capsule_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void Input_Move(const FInputActionValue& Value);
	void Input_Dash();
	void Input_Interact();
	void Input_PickupOrDrop();
	void Input_Throw();

	UFUNCTION(Server, Reliable)
	void Server_Interact();

	UFUNCTION(Server, Reliable)
	void Server_Pickup();

	UFUNCTION(Server, Reliable)
	void Server_Drop();

	UFUNCTION(Server, Reliable)
	void Server_Throw();

private:

	struct FOverlappedActor
	{
		AActor* Actor;
		uint64 NbOccurrences;
	};

	TArray<FOverlappedActor> OverlappedActors;

	TScriptInterface<IInteractable> BestInteractable;
	TWeakObjectPtr<UCarriableComponent> BestCarriable;
};
