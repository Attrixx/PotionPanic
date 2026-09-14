// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <PhysicsEngine/PhysicalAnimationComponent.h>
#include "AlchemistCustomizationAsset.h"
#include "ActivityInputCapture.h"
#include "AlchemistBase.generated.h"

enum class EActivityInputSlot : uint8;
class UHolderComponent;
class URangeComponent;
class UPhysicalAnimationComponent;
class UInputMappingContext;
class UInputAction;
class ULocalPlayer;
class UInteractableActorFilter;
class UCarriableActorFilter;
class UFreeHolderActorFilter;
class UActivityExecutor;
class USoundBase;
class UNetworkSoundComponent;
struct FInputActionValue;


UCLASS(Abstract)
class PLAYER_API AAlchemistBase : public ACharacter, public IActivityInputCapture
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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
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

	/**
	 * Matches anything the player can pick up: a loose item, or an occupied holder (a station, or
	 * another player) willing to give up what it has. Both go through this one filter so the range
	 * component's priority ranking applies uniformly to either kind.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Actor Filter")
	TObjectPtr<UCarriableActorFilter> CarriableFilter;

	/** Items are never left on the ground: putting one down means finding a holder to put it on. */
	UPROPERTY(BlueprintReadOnly, Category = "Actor Filter")
	TObjectPtr<UFreeHolderActorFilter> FreeHolderFilter;

#if WITH_EDITORONLY_DATA
	/**
	 * Tracks InteractableFilter, CarriableFilter and FreeHolderFilter with RangeComponent, even
	 * though they are otherwise only ever queried one-shot via FindBestMatchingActor. Tracking is
	 * what feeds RangeComponent's own bShowDebugBestActors display, so this is what puts these
	 * filters' best match on screen alongside it.
	 */
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugTrackActorFilters = false;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	FName RagdollRootBoneName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	FPhysicalAnimationData PhysicalAnimationData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNetworkSoundComponent> NetworkSoundComponent;

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> ComboMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SlotRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> CancelAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
	float ThrowForce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> DashSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> DropSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> ThrowSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> CollideSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> StunSound;

public:

	// IActivityInputCapture
	void BeginActivityInputCapture_Implementation(UActivityExecutor* Executor) override;
	void EndActivityInputCapture_Implementation() override;

private:

	void SetActorCustomDepthEnabled(AActor* TargetActor, bool bEnabled, int32 StencilValue = 9);

	/** Plays Sound on this machine and relays it to the others, crediting this pawn as instigator. */
	int32 PlayNetworkedSound(USoundBase* Sound);

	/**
	 * Turns a pickup candidate into the Carriable it stands for: itself when it is one, otherwise
	 * whatever the holder it hangs off is offering.
	 * @return The Carriable, or null when there is nothing to take.
	 */
	static UObject* ResolveCarriable(AActor* Candidate);

	/** @return The local player this pawn's input runs on. Null where the pawn is not played. */
	ULocalPlayer* GetInputLocalPlayer() const;

	/**
	 * Applies or lifts one mapping context, remapped through the keybind subsystem so a key the
	 * player rebound still reaches the same action.
	 * @param LocalPlayer Whose input stack to change. Nothing happens when null.
	 * @param Context The context to apply or lift. Nothing happens when null.
	 * @param bActive True to apply it, false to lift it.
	 * @param Priority Priority to apply it at. Ignored when lifting.
	 */
	static void SetMappingContextActive(ULocalPlayer* LocalPlayer, UInputMappingContext* Context, bool bActive, int32 Priority);

	/** Takes the capture flag over on the authority and runs the local half of it there too. */
	void SetActivityInputCaptured(bool bCaptured);

	/** Pushes or pops the combo mapping context. Does nothing where this pawn is not played. */
	UFUNCTION()
	void OnRep_ActivityInputCaptured();

private: // Input

	void Input_Move(const FInputActionValue& Value);
	void Input_Dash();

	void Input_Interact();
	void Input_PickupOrDrop();
	void Input_Throw();

	void Input_ActivityUp();
	void Input_ActivityLeft();
	void Input_ActivityDown();
	void Input_ActivityRight();
	void Input_ActivityCancel();

	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* Interactable);

	/** Takes what Candidate offers: itself if it is a loose Carriable, or whatever occupies its holder. */
	UFUNCTION(Server, Reliable)
	void Server_Pickup(AActor* Candidate);

	/** Hands the carried item over to Receiver's holder. Nothing happens if it cannot take it. */
	UFUNCTION(Server, Reliable)
	void Server_Place(AActor* Receiver);

	UFUNCTION(Server, Reliable)
	void Server_Throw(FVector Direction);
	
	UFUNCTION(Server, Reliable)
	void Server_ActivityInput(EActivityInputSlot Slot);

	UFUNCTION(Server, Reliable)
	void Server_ActivityCancel();

private: // Activity input capture

	/** Executor whose step took our inputs. Authority side only. */
	TWeakObjectPtr<UActivityExecutor> CapturingExecutor;

	/** True while a step owns this pawn's inputs. Owner only. */
	UPROPERTY(ReplicatedUsing = OnRep_ActivityInputCaptured)
	bool bActivityInputCaptured = false;
};
