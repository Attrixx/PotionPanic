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

UENUM(BlueprintType)
enum class EStationCancelPolicy : uint8
{
	KeepConsumed UMETA(DisplayName = "KeepConsumed"),
	RequeueInstruction UMETA(DisplayName = "RequeueInstruction")
};

UENUM(BlueprintType)
enum class EStationRuntimeError : uint8
{
	None UMETA(DisplayName = "None"),
	MissingWorldContext UMETA(DisplayName = "MissingWorldContext"),
	MissingOutputAsset UMETA(DisplayName = "MissingOutputAsset"),
	OutputSpawnFailed UMETA(DisplayName = "OutputSpawnFailed")
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

	UFUNCTION(BlueprintPure, Category = "Station|Instructions")
	bool HasBufferedBatch() const { return BufferedInputCount > 0; }

	UFUNCTION(BlueprintCallable, Category = "Station|Instructions")
	void ResetBufferedBatch();

	UFUNCTION(BlueprintCallable, Category = "Station|Failure")
	bool ApplyFailureOutcome(const FPrimaryAssetId& FailureOutputItem, int32 FailureOutputQuantity, bool bConsumeHeldItem, bool bClearInstructionQueue = true);

	UFUNCTION(BlueprintPure, Category = "Station|Failure")
	bool HasFailureOutputOverride() const { return bUseStationFailureOutputOverride && StationFailureOutputItem.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Station|Failure")
	FPrimaryAssetId GetFailureOutputOverrideItem() const { return StationFailureOutputItem; }

	UFUNCTION(BlueprintPure, Category = "Station|Failure")
	int32 GetFailureOutputOverrideQuantity() const { return FMath::Max(1, StationFailureOutputQuantity); }

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
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
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
	void OnBufferedBatchTimeout();
	void StopExecutionTimers();
	void StopBufferedBatchTimer();
	void StopPendingOutputRetryTimer();
	void SetStationState(EStationState NewState);
	void ReportRuntimeError(EStationRuntimeError ErrorCode, const FText& Message);
	bool TryResolveInstructionForItem(const FPrimaryAssetId& ItemId, FInstruction& OutInstruction);
	const UInteractionDefinitionAsset* ResolveInteractionDefinition(const FInstruction& Instruction) const;
	bool ConsumeCarriable(UCarriableComponent* Carriable) const;
	bool SpawnInstructionOutput(const FInstruction& Instruction);
	void TrySpawnPendingOutput();
	void ResetCurrentInstructionState();
	int32 GetRequiredInputCount(const FInstruction& Instruction) const;
	int32 GetRequiredOutputCount(const FInstruction& Instruction) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StationMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	/** Optional socket used by ItemHolder attachment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Slots")
	FName ItemSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	TArray<TObjectPtr<UActivityAsset>> Activities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	TArray<FInstruction> PossibleInstructions;

	/** Optional activity -> interaction mapping used by QTE/IFT stations. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Interaction")
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

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	int32 BufferedInputCount = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	int32 PendingOutputCount = 0;

	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Station|State")
	FPrimaryAssetId PendingOutputItem;

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnBufferedBatchResetBP(int32 DiscardedInputCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Station|Visuals")
	void OnStationRuntimeErrorBP(EStationRuntimeError ErrorCode, const FText& Message);

protected:
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CurrentInstigator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Rules")
	float InteractionDistance = 200.0f;

	/** Optional allow-list for accepted input item ids (empty = no id restriction). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Filters")
	TArray<FPrimaryAssetId> AllowedInputItems;

	/** Optional required item data tags for accepted inputs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Filters")
	TArray<FName> RequiredInputDataTags;

	/** Tag-matching mode for RequiredInputDataTags. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Filters")
	bool bRequireAllInputDataTags = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Rules", meta = (ClampMin = "0.01"))
	float ExecutionTickInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Rules", meta = (ClampMin = "0.1"))
	float BufferedBatchTimeoutSeconds = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Rules")
	EStationCancelPolicy CancelPolicy = EStationCancelPolicy::KeepConsumed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Failure")
	bool bUseStationFailureOutputOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Failure", meta = (EditCondition = "bUseStationFailureOutputOverride"))
	FPrimaryAssetId StationFailureOutputItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Failure", meta = (ClampMin = "1", EditCondition = "bUseStationFailureOutputOverride"))
	int32 StationFailureOutputQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Rules", meta = (ClampMin = "0.0"))
	float PendingOutputRetryDelaySeconds = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station|Rules", meta = (ClampMin = "0"))
	int32 MaxPendingOutputSpawnRetries = 3;

	FTimerHandle ProcessingTimer;
	FTimerHandle ExecutionTickTimer;
	FTimerHandle BufferedBatchTimer;
	FTimerHandle PendingOutputRetryTimer;

	int32 PendingOutputRetryCount = 0;

	UPROPERTY(Transient)
	TObjectPtr<UInteractionBase> ActiveInteraction;
};
