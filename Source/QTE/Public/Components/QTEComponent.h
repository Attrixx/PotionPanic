#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Replication/QTEAuthoritySession.h"
#include "Input/QTEInputBinder.h"
#include "Runtime/QTEResolver.h"
#include "Runtime/QTERuntime.h"
#include "Core/QTETypes.h"
#include "QTEComponent.generated.h"

class UEnhancedInputComponent;
class UInputAction;
class UQTEDefinitionDataAsset;
struct FInputActionInstance;
class UInputMappingContext;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQTEStateChanged, const FQTERuntimeState&, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQTEFinished, const FQTEResult&, Result);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnQTEAuthorityFinished, int32, const FQTEAuthorityResult&);

UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class QTE_API UQTEComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQTEComponent();
	~UQTEComponent() override;

	void InitializeEnhancedInput(UEnhancedInputComponent* InEnhancedInputComponent);

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
	void NotifyClientStartAuthorityQTE(int32 RequestId, UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor);
	void NotifyClientCancelAuthorityQTE(int32 RequestId);
	void NotifyClientCompleteAuthorityQTE(int32 RequestId, const FQTEAuthorityResult& AuthorityResult);
	void NotifyServerConfirmAuthorityQTEReady(int32 RequestId);
	void NotifyServerAuthorityQTEStartFailed(int32 RequestId, const FText& FailureMessage);
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
	void DrawMashDebugFeedback() const;
	bool ShouldForwardAuthorityInput() const;
	bool ShouldDeferFinishToAuthority() const;
	void ClearActiveSessionReferences();

	UFUNCTION(Client, Reliable)
	void ClientStartAuthorityQTE(int32 RequestId, UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor);

	UFUNCTION(Client, Reliable)
	void ClientCancelAuthorityQTE(int32 RequestId);

	UFUNCTION(Client, Reliable)
	void ClientCompleteAuthorityQTE(int32 RequestId, const FQTEAuthorityResult& AuthorityResult);

	UFUNCTION(Server, Reliable)
	void ServerConfirmAuthorityQTEReady(int32 RequestId);

	UFUNCTION(Server, Reliable)
	void ServerNotifyAuthorityQTEStartFailed(int32 RequestId, const FText& FailureMessage);

	UFUNCTION(Server, Unreliable)
	void ServerSubmitAuthorityPressedInput(int32 RequestId, int32 StepIndex);

	UFUNCTION(Server, Unreliable)
	void ServerSubmitAuthorityReleasedInput(int32 RequestId, int32 StepIndex);

private:
	TObjectPtr<UQTEDefinitionDataAsset> ActiveDefinition = nullptr;

	FQTERuntimeState RuntimeState;

	FQTEResult LastResult;

	FQTEAuthoritySession AuthoritySession;
	FQTEInputBinder InputBinder;
	TUniquePtr<FQTERuntime> ActiveQTERuntime;
	TWeakObjectPtr<UObject> SourceObject;
	TWeakObjectPtr<AActor> InstigatorActor;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|Authority", meta = (ClampMin = 0.1))
	float AuthorityMirrorReadyTimeoutSeconds = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|Debug")
	bool bDrawMashDebugFeedback = false;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|UI", meta = (ClampMin = 0.0))
	float MashDebugBarWidth = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "QTE|UI", meta = (ClampMin = 0.0))
	float MashDebugBarHeightOffset = 110.f;

	float HoldStartTime = 0.f;
};
