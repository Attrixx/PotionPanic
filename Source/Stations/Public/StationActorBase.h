#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "Instruction.h"
#include "InteractionBase.h"
#include "StationActorBase.generated.h"

class UActivityAsset;
class UStaticMeshComponent;
class AItemActor;
class APlayerController;
class UCarriableComponent;
class UHolderComponent;
class UInteractionBase;
class UInteractionDefinitionAsset;

/**
 * Base class for all station actors.
 *
 * Stations execute and consume instructions.
 * They do not know about recipes, game modes, game flow, or UI logic.
 */
UENUM(BlueprintType)
enum class EStationState : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Processing UMETA(DisplayName = "Processing"),
	Completed UMETA(DisplayName = "Completed")
};

USTRUCT(BlueprintType)
struct STATIONS_API FStationActivityInteraction
{
	GENERATED_BODY()

	/** Activity key used to map an instruction to an interaction definition. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station|Interaction")
	TSoftObjectPtr<UActivityAsset> Activity;

	/** Runtime interaction to execute for this activity (QTE/IFT/etc). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station|Interaction")
	TObjectPtr<UInteractionDefinitionAsset> InteractionDefinition = nullptr;
};

UCLASS()
class STATIONS_API AStationActorBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AStationActorBase();

	void Interact(APlayerController& InInstigator) override;

	/** Adds one instruction to the execution queue. */
	UFUNCTION(BlueprintCallable, Category = "Station|Instructions")
	void QueueInstruction(const FInstruction& InInstruction);

	/** Compatibility helper: resets queue to exactly one instruction. */
	UFUNCTION(BlueprintCallable, Category = "Station|Instructions")
	void SetInstruction(const FInstruction& InInstruction);

	UFUNCTION(BlueprintCallable, Category = "Station|Instructions")
	void ClearInstructionQueue();

	UFUNCTION(BlueprintPure, Category = "Station|Instructions")
	int32 GetQueuedInstructionCount() const { return InstructionQueue.Num(); }

	/** For QTE/IFT stations: push one interaction attempt result. */
	UFUNCTION(BlueprintCallable, Category = "Station|Interaction")
	void SubmitInteractionAttempt(bool bSuccess);

	UFUNCTION(BlueprintPure, Category = "Station")
	TArray<UActivityAsset*> GetActivities() const;

	/**
	 * Checks if a specific item can be placed on this station.
	 * Default implementation allows any valid item id.
	 */
	virtual bool CanPlaceItem(const FPrimaryAssetId& ItemId) const;

	/**
	 * Checks if the instruction is valid for this station.
	 */
	virtual bool CanExecuteInstruction(const FInstruction& Instruction) const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Starts processing the instruction currently consumed by the station. */
	virtual void StartProcessing(const FInstruction& Instruction);

	/** Finishes processing and handles input consumption/output spawn. */
	virtual void FinishProcessing();

	/** Cancels current processing logic. */
	virtual void CancelProcessing();

	UFUNCTION()
	void OnRep_StationState();

	UFUNCTION()
	void HandleInteractionFinished(FInteractionOutput InteractionOutput);

	void OnExecutionTick();
	void OnProcessingTimerFinished();
	void StopExecutionTimers();
	void SetStationState(EStationState NewState);
	bool TryResolveInstructionForItem(const FPrimaryAssetId& ItemId, FInstruction& OutInstruction);
	const UInteractionDefinitionAsset* ResolveInteractionDefinition(const FInstruction& Instruction) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StationMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	/** Optional socket used by ItemHolder attachment. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station|Slots")
	FName ItemSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station")
	TArray<TObjectPtr<UActivityAsset>> Activities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station")
	TArray<FInstruction> PossibleInstructions;

	/** Optional activity -> interaction mapping used by QTE/IFT stations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station|Interaction")
	TArray<FStationActivityInteraction> ActivityInteractions;

	/** Runtime queue of external instructions to be consumed by this station. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	TArray<FInstruction> InstructionQueue;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	FInstruction CurrentInstruction;

	UPROPERTY(ReplicatedUsing = OnRep_StationState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	EStationState StationState = EStationState::Idle;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	float ProcessingStartTime = 0.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	float ProcessingDuration = 0.0f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnInstructionQueuedBP(const FInstruction& Instruction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnInstructionConsumedBP(const FInstruction& Instruction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnStartProcessingBP(const FInstruction& Instruction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnFinishProcessingBP(const FInstruction& Instruction);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnCancelProcessingBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnStationStateChangedBP(EStationState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnInteractionResolvedBP(FInteractionOutput InteractionOutput);

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CurrentInstigator;

	UPROPERTY(EditDefaultsOnly, Category = "Station|Rules")
	float InteractionDistance = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Station|Rules", meta = (ClampMin = "0.01"))
	float ExecutionTickInterval = 0.1f;

	FTimerHandle ProcessingTimer;
	FTimerHandle ExecutionTickTimer;

	UPROPERTY(Transient)
	TObjectPtr<UInteractionBase> ActiveInteraction;
};
