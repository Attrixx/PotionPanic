// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActivityExecutionState.h"
#include "ActivityStepPresentation.h"
#include "ActivityExecutor.generated.h"

class UActivityAsset;
class UActivityStep;
class UActivityEvaluator;
class UActivityConclusion;
struct FActivityStepResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActivityExecutionStatusChangedDelegate, UActivityExecutor*, Executor, EActivityExecutionStatus, NewStatus);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FActivityStepPresentationChangedDelegate, UActivityExecutor*, Executor);

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActivityInputSlot : uint8
{
	None  = 0 UMETA(Hidden),
	Up    = 1 << 0,
	Left  = 1 << 1,
	Down  = 1 << 2,
	Right = 1 << 3,
};

/**
 * 
 */
UCLASS(meta=(BlueprintSpawnableComponent))
class ACTIVITIES_API UActivityExecutor : public UActorComponent
{
	GENERATED_BODY()

public:

	UActivityExecutor();

protected:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	/**
	 * Initializes the executor with a holder component.
	 * If the executor was already bound to another Holder, the previous binding is cleared.
	 * @param HolderComponent The component responsible for holding the interactive object. Must be valid.
	 */
	UFUNCTION(BlueprintCallable)
	void Initialize(UHolderComponent* HolderComponent);

	/**
	 * Starts the execution of a new activity from a definition asset.
     * Automatically cancel any currently ongoing activity.
	 * @param Activity Activity The asset defining the activity structure (Steps, Evaluator, Conclusion).
	 * @param Instigator The optional actor that initiated the activity.
	 * @param bItemTakenFromInstigator Whether the item on the holder was taken out of the
	 *        instigator's hands to start this activity, rather than already sitting there.
	 * @param bItemRolesSwapped Whether a Swappable activity matched with the station's and the
	 *        instigator's items the other way around.
	 */
	UFUNCTION(BlueprintCallable)
	void StartActivity(UActivityAsset* Activity, AActor* Instigator = nullptr, bool bItemTakenFromInstigator = false,
	                   bool bItemRolesSwapped = false);

	/**
	 * Forwards an interaction event to the currently executing activity step.
	 * Does nothing if there is no ongoing activity.
	 * The current step is responsible for handling the internal logic triggered by this interaction.
	 * @param Instigator The actor triggering the interaction.
	 */
	UFUNCTION(BlueprintCallable)
	void Interact(AActor* Instigator);

	/**
	 * Interrupts the ongoing activity and resets the executor's state.
	 * Propagates the cancellation to the current step to allow it to execute its cleanup logic.
	 */
	UFUNCTION(BlueprintCallable)
	void Cancel();

	UFUNCTION(BlueprintCallable)
	EActivityExecutionStatus GetExecutionStatus() const;

	/**
	 * Forwards a captured input to the current step.
	 * @param Instigator Actor the press came from.
	 * @param Slot The input that was pressed.
	 */
	UFUNCTION(BlueprintCallable)
	void ReceiveActivityInput(AActor* Instigator, EActivityInputSlot Slot);

	UFUNCTION(BlueprintCallable)
	double GetServerTimeSeconds() const;

	template <typename T>
		requires std::derived_from<std::remove_cvref_t<T>, FActivityStepPresentationCustomInfo>
	void SetStepPresentationCustomInfo(T&& InInfo)
	{
		using FInfoType = std::remove_cvref_t<T>;
		if (IsAuthority())
		{
			if (Presentation.CustomInfo.GetScriptStruct() == FInfoType::StaticStruct())
				Presentation.CustomInfo.GetMutable<FInfoType>() = Forward<T>(InInfo);
			else
				Presentation.CustomInfo.InitializeAs<FInfoType>(Forward<T>(InInfo));
			UpdatePresentation();
		}
	}

	/** Takes the current presentation down. Authority only. */
	void ClearStepPresentation();

	/** Context of the last started activity: holder, item, instigator, status and score. */
	const FActivityExecutionState& GetExecutionState() const { return State; }

	/** What the running step wants displayed. StepClass is null when there is nothing to draw. */
	UFUNCTION(BlueprintPure)
	const FActivityStepPresentation& GetStepPresentation() const { return Presentation; }

	UPROPERTY(BlueprintAssignable)
	FActivityExecutionStatusChangedDelegate OnExecutionStatusChanged;

	/** Fires on every side whenever GetStepPresentation() changes, authority included. */
	UPROPERTY(BlueprintAssignable)
	FActivityStepPresentationChangedDelegate OnStepPresentationChanged;

private:

	/**
	 * @return True where the activity actually runs. Everywhere else this component is a replicated
	 *         view: State and Presentation are filled by the network, and Steps is empty.
	 */
	bool IsAuthority() const;

	UFUNCTION()
	void Holder_OnCarriableChanged(UHolderComponent* Holder);

	/**
	 * Points the state at Instigator and re-reads its holder and carried item. Called on start and
	 * on every interact: the instigator may have swapped, dropped or consumed what it carries, and
	 * a pointer kept from the previous call would go stale.
	 */
	void RefreshInstigator(AActor* Instigator);
	void ContinueExecution();
	void OnStepFinished(const FActivityStepResult& Result);
	void Conclude(EActivityExecutionStatus Status); // Conclude and broadcast the status change
	void Reset(EActivityExecutionStatus Status); // Reset and broadcast the status change

	UFUNCTION()
	void OnRep_State(const FActivityExecutionState& OldState);

	void UpdatePresentation();

	UFUNCTION()
	void OnRep_Presentation();

private:

	UPROPERTY(ReplicatedUsing=OnRep_State)
	FActivityExecutionState State;

	UPROPERTY(ReplicatedUsing=OnRep_Presentation)
	FActivityStepPresentation Presentation;

	UPROPERTY()
	TArray<TObjectPtr<UActivityStep>> Steps;

	UPROPERTY()
	TObjectPtr<UActivityEvaluator> Evaluator;

	UPROPERTY()
	TObjectPtr<UActivityConclusion> Conclusion;

	int32 CurrentStepIndex = 0;

	// Guards against calling CancelStep before StartStep.
	// Must be reset when writing to CurrentStepIndex.
	bool bCurrentStepStarted = false;
};
