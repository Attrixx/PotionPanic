// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <PhysicsEngine/PhysicalAnimationComponent.h>
#include "AlchemistCustomizationAsset.h"
#include "AlchemistBase.generated.h"

class UHolderComponent;
class URangeComponent;
class UPhysicalAnimationComponent;
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

public:

	UFUNCTION(BlueprintImplementableEvent)
	void SetColor(FColor Color);

	/**
	 * Sets the Custom Depth Stencil value used to visually distinguish players.
	 * Convention: 0 = disabled, 1-4 = Player 0-3.
	 */
	void SetPlayerStencilIndex(int32 StencilValue);

	UFUNCTION(BlueprintCallable, Category = "Customization")
	void ApplyCustomization(USkeletalMesh* NewMesh, FColor NewColor);

	UFUNCTION(BlueprintCallable)
	bool IsCarrying() const;

	UFUNCTION(BlueprintCallable)
	void ApplyStunRagdoll();

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	FName RagdollRootBoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	FPhysicalAnimationData PhysicalAnimationData;

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

	// Timer interval for sorting items in range (computes difference in locations)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay", meta=(ClampMin=0, UIMax=1))
	float InRangeInfosSortInterval = 0.1f;

private:

	void SetActorCustomDepthEnabled(AActor* TargetActor, bool bEnabled, int32 StencilValue = 9);

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

private:
	
	FTimerHandle InRangeSortTimerHandle;

	AActor* LastBestInteractable;
	AActor* LastBestCarriable;

};
