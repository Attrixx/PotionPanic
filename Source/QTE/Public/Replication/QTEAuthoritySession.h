#pragma once

#include "CoreMinimal.h"
#include "Core/QTETypes.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "UObject/StrongObjectPtr.h"

class UQTEComponent;
class UQTEDefinitionDataAsset;
class AActor;

class QTE_API FQTEAuthoritySession
{
public:
	FQTEAuthoritySession();
	explicit FQTEAuthoritySession(UQTEComponent& InOwnerComponent);
	~FQTEAuthoritySession();

	void Initialize(UQTEComponent& InOwnerComponent);
	int32 Start(UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor);
	void Cancel(int32 RequestId);
	void HandleEndPlay();
	void HandleMirrorReadyTimeout();
	void Resolve(const FQTEAuthorityResult& AuthorityResult);
	void HandleClientStart(int32 RequestId, UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor);
	void HandleClientCancel(int32 RequestId);
	void HandleClientComplete(int32 RequestId, const FQTEAuthorityResult& AuthorityResult);
	void HandleServerConfirmReady(int32 RequestId);
	void HandleServerStartFailed(int32 RequestId, const FText& FailureMessage);
	void HandleServerPressedInput(int32 RequestId, int32 StepIndex);
	void HandleServerReleasedInput(int32 RequestId, int32 StepIndex);

	bool ShouldForwardInput() const;
	bool ShouldDeferFinish() const;
	int32 GetActiveRequestId() const;

private:
	UQTEComponent& GetOwnerComponent() const;
	void StartMirrorReadyTimeout(int32 RequestId);
	void ClearMirrorReadyTimeout();
	void FailRequest(int32 RequestId, const FText& FailureMessage, bool bNotifyOwningClient);

private:
	UQTEComponent* OwnerComponent = nullptr;
	// Strong ref: the session is a plain C++ object (not a UObject/USTRUCT), so the GC
	// cannot see this pointer. Without it, a GC pass during the start round-trip
	// (Client_StartAuthorityQTE -> Server_ConfirmAuthorityQTEReady) could collect the
	// definition and leave a dangling pointer.
	TStrongObjectPtr<UQTEDefinitionDataAsset> PendingDefinition;
	TWeakObjectPtr<AActor> PendingSourceActor;
	FTimerHandle MirrorReadyTimeoutHandle;
	int32 NextRequestId = 1;
	int32 ActiveRequestId = INDEX_NONE;
};
