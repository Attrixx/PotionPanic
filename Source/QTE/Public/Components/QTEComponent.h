#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputBindable.h"
#include "Replication/QTEAuthoritySession.h"
#include "Runtime/QTERuntime.h"
#include "Core/QTETypes.h"
#include "QTEComponent.generated.h"

class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UQTEDefinitionDataAsset;
struct FInputActionInstance;
class UInputMappingContext;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQTEStateChanged, const FQTERuntimeState&, State);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQTEFinished, const FQTEResult&, Result);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQTEAuthorityFinished, int32, const FQTEAuthorityResult&);

UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class QTE_API UQTEComponent : public UActorComponent, public IInputBindable
{
	GENERATED_BODY()

public:

	UQTEComponent();
	~UQTEComponent() override;

	void SetupInputComponent_Implementation(UEnhancedInputComponent* EIC) override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "QTE")
	int32 StartAuthorityQTE(UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor = nullptr);

	void CancelAuthorityQTE(int32 RequestId);

	UFUNCTION(BlueprintCallable, Category = "QTE")
	void CancelQTE();

	UFUNCTION(BlueprintCallable, Category = "QTE")
	void InterruptQTE();

	UFUNCTION(BlueprintPure, Category = "QTE")
	bool IsQTERunning() const;

	UFUNCTION(BlueprintPure, Category = "QTE")
	FQTERuntimeState GetCurrentQTEState() const;

	UFUNCTION(BlueprintPure, Category = "QTE")
	FQTEStepDefinition GetCurrentQTEStep() const;

	UFUNCTION(BlueprintPure, Category = "QTE")
	UQTEDefinitionDataAsset* GetCurrentQTEDefinition() const;

	UFUNCTION(BlueprintPure, Category = "QTE")
	FQTEResult GetLastQTEResult() const;

	UFUNCTION(BlueprintCallable, Category = "QTE|Input")
	bool SubmitInputPressed(const UInputAction* InputAction);

	UFUNCTION(BlueprintCallable, Category = "QTE|Input")
	bool SubmitInputReleased(const UInputAction* InputAction);

	UFUNCTION(BlueprintCallable, Category = "QTE|Input")
	bool SubmitInputTriggered(const UInputAction* InputAction);

	UPROPERTY(BlueprintAssignable, Category = "QTE")
	FOnQTEStateChanged OnQTEStarted;

	UPROPERTY(BlueprintAssignable, Category = "QTE")
	FOnQTEStateChanged OnQTEStepChanged;

	UPROPERTY(BlueprintAssignable, Category = "QTE")
	FOnQTEStateChanged OnQTEUpdated;

	UPROPERTY(BlueprintAssignable, Category = "QTE")
	FOnQTEFinished OnQTEFinished;

	FOnQTEAuthorityFinished OnQTEAuthorityFinished;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	// Semi-public interface (used by FQTEAuthoritySession / FQTERuntime)
	bool StartQTEInternal(UQTEDefinitionDataAsset* InDefinition, UObject* InSourceObject = nullptr, AActor* InInstigator = nullptr);
	const FQTEStepDefinition* GetCurrentStep() const;
	UQTEDefinitionDataAsset* GetActiveDefinition() const;
	UInputMappingContext* GetInputMappingContext() const;
	FQTERuntimeState& GetMutableRuntimeState();
	const FQTERuntimeState& GetRuntimeState() const;
	float& GetMutableHoldStartTime();
	void FinishQTE(EQTEState FinalState, const FText& OverrideMessage = FText());
	void RefreshRuntimeState();
	void BroadcastQTEStepChanged();
	void BroadcastQTEUpdated();
	void ApplyAuthorityCompletionToLocalMirror(const FQTEAuthorityResult& AuthorityResult);
	float GetAuthorityMirrorReadyTimeoutSeconds() const;
	float GetDifficultyScale() const;
	void HandleAuthorityMirrorReadyTimeout();
	void HandleEnhancedInputStarted(const FInputActionInstance& Instance);
	void HandleEnhancedInputTriggered(const FInputActionInstance& Instance);
	void HandleEnhancedInputCompleted(const FInputActionInstance& Instance);
	void HandleEnhancedInputCanceled(const FInputActionInstance& Instance);

protected:

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	UObject* ResolveSourceObject(UObject* InSourceObject) const;
	bool ShouldBindLocalInput() const;
	bool ShouldForwardAuthorityInput() const;
	bool ShouldDeferFinishToAuthority() const;
	void DrawMashDebugFeedback() const;
	void ClearActiveSessionReferences();

	void BindInputForDefinition();
	void UnbindInput();
	void AddInputMappingContext();
	void RemoveInputMappingContext();
	bool HasEnhancedInputComponent() const;
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

private:

	friend FQTEAuthoritySession;

	UFUNCTION(Client, Reliable)
	void Client_StartAuthorityQTE(int32 RequestId, UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor);
	
	UFUNCTION(Client, Reliable)
	void Client_CancelAuthorityQTE(int32 RequestId);
	
	UFUNCTION(Client, Reliable)
	void Client_CompleteAuthorityQTE(int32 RequestId, const FQTEAuthorityResult& AuthorityResult);
	
	UFUNCTION(Server, Reliable)
	void Server_ConfirmAuthorityQTEReady(int32 RequestId);
	
	UFUNCTION(Server, Reliable)
	void Server_NotifyAuthorityQTEStartFailed(int32 RequestId, const FText& FailureMessage);
	
	// Reliable: these are discrete, low-frequency input events. A dropped packet
	// would desync the authority mirror (missed mash tick / never-registered press),
	// causing unfair failures, so reliability is worth the negligible cost.
	UFUNCTION(Server, Reliable)
	void Server_SubmitAuthorityPressedInput(int32 RequestId, int32 StepIndex);

	UFUNCTION(Server, Reliable)
	void Server_SubmitAuthorityReleasedInput(int32 RequestId, int32 StepIndex);

private:

	struct FBoundInputHandles
	{
		uint32 StartedHandle = 0;
		uint32 TriggeredHandle = 0;
		uint32 CompletedHandle = 0;
		uint32 CanceledHandle = 0;
	};

	/**
	 * Marks that an FQTERuntime method is on the stack. FinishQTE is reachable from inside one
	 * (a mistake, or the last step completing) and clears ActiveQTERuntime, which would free the
	 * object whose method is still running. While this scope is open that release is deferred to
	 * RuntimePendingRelease and happens once the outermost runtime call returns.
	 */
	struct FRuntimeCallScope
	{
		explicit FRuntimeCallScope(UQTEComponent& InOwner);
		~FRuntimeCallScope();

	private:
		UQTEComponent& Owner;
	};

	TObjectPtr<UQTEDefinitionDataAsset> ActiveDefinition = nullptr;
	FQTERuntimeState RuntimeState;
	FQTEResult LastResult;
	FQTEAuthoritySession AuthoritySession;
	TUniquePtr<FQTERuntime> ActiveQTERuntime;
	TUniquePtr<FQTERuntime> RuntimePendingRelease;
	int32 RuntimeCallDepth = 0;
	TWeakObjectPtr<UObject> SourceObject;
	TWeakObjectPtr<AActor> InstigatorActor;

	TWeakObjectPtr<UEnhancedInputComponent> EnhancedInputComponent;
	TMap<TObjectPtr<UInputAction>, FBoundInputHandles> BoundInputHandles;
	bool bAddedMappingContext = false;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|Authority", meta = (ClampMin = 0.1))
	float AuthorityMirrorReadyTimeoutSeconds = 1.0f;

	/**
	 * Backstop for a definition that configures no timeout at all. Gameplay input is gated on
	 * IsQTERunning() (AAlchemistBase::ShouldBlockGameplayInput), so a QTE that can never end
	 * locks the player out of the game for good. Data validation rejects such definitions; this
	 * catches anything that reaches a cooked build anyway. Set to 0 to disable.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "QTE|Safety", meta = (ClampMin = 0.0))
	float FailsafeTimeoutSeconds = 30.f;

	/**
	 * Stacks on top of each definition's own DifficultyMultiplier, so game progression can tighten
	 * every QTE this player runs without touching a single asset. 1 leaves definitions as authored.
	 * Set it from Blueprint when a round starts; it is read each time the effective timings are
	 * recomputed, so a change lands immediately.
	 *
	 * Only the authority's value decides an outcome: the client mirror is there for responsiveness
	 * and its result is overwritten by ApplyAuthorityCompletionToLocalMirror. A scale that differs
	 * between the two is therefore cosmetic, not a desync, but it will look wrong to the player.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "QTE|Difficulty",
		meta = (ClampMin = 0.1, AllowPrivateAccess = "true"))
	float DifficultyScale = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|Debug")
	bool bDrawMashDebugFeedback = false;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|UI", meta = (ClampMin = 0.0))
	float MashDebugBarWidth = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|UI", meta = (ClampMin = 0.0))
	float MashDebugBarHeightOffset = 110.f;

	float HoldStartTime = 0.f;
};
