#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ActivityAsset.h"
#include "Instruction.h"
#include "StationActorBase.generated.h"

class UStaticMeshComponent;

class AItemActor;
class APlayerController;
class UHolderComponent;

/**
 * Base class for all station actors.
 * 
 * Stations are interactable actors that support specific activities.
 * All station logic is handled through the Interact() method and an external manager.
 */
UENUM(BlueprintType)
enum class EStationState : uint8
{
	Idle,
	Processing,
	Completed
};

UCLASS()
class STATIONS_API AStationActorBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AStationActorBase();

	void Interact(APlayerController& InInstigator) override;

	UFUNCTION(BlueprintCallable, Category = "Station")
	void SetInstruction(const FInstruction& InInstruction) { CurrentInstruction = InInstruction; }

	const TArray<UActivityAsset*>& GetActivities() const { return Activities; }

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StationMesh;

	// TODO (Nath): Configure socket for item placement in Blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station")
	TArray<UActivityAsset*> Activities;

	// TODO (Nath): Configure in Blueprint which instructions this station can execute
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station")
	TArray<FInstruction> PossibleInstructions;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	FInstruction CurrentInstruction;

	UPROPERTY(ReplicatedUsing = OnRep_StationState, VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	EStationState StationState = EStationState::Idle;

	UFUNCTION()
	void OnRep_StationState();

	/** Time when processing started (server time). Used for calculating progress. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	float ProcessingStartTime;

	/** Total duration of current processing. */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	float ProcessingDuration;

	FTimerHandle ProcessingTimer;

	UFUNCTION()
	void OnProcessingTimerFinished();

public:
	/**
	 * Checks if a specific item can be placed on this station.
	 * Default implementation allows anything if station is empty.
	 * Override for specific stations (e.g. Cauldron only accepts ingredients).
	 */
	virtual bool CanPlaceItem(const UItemAsset* Item) const;
	
	/**
	 * Checks if the instruction is valid for this station (e.g. check Logic/Activity).
	 */
	virtual bool CanExecuteInstruction(const FInstruction& Instruction) const;

	virtual void Tick(float DeltaTime) override;

protected:
	/**
	 * Starts processing the current instruction.
	 */
	virtual void StartProcessing(const FInstruction& Instruction);

	/**
	 * Called when processing finishes.
	 * Default implementation destroys input and spawns output.
	 */
	virtual void FinishProcessing();


	/**
	 * Cancels the current processing logic (e.g. if player moves away).
	 */
	virtual void CancelProcessing();
	
	// Visual Hooks
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnStartProcessingBP(const FInstruction& Instruction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnFinishProcessingBP(const FInstruction& Instruction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnCancelProcessingBP();

	/** Called on Server (State Change) and Client (OnRep) to update visuals. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnStationStateChangedBP(EStationState NewState);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CurrentInstigator;

	UPROPERTY(EditDefaultsOnly, Category = "Station")
	float InteractionDistance = 200.0f;
};
