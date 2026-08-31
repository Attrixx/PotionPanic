#pragma once

#include "CoreMinimal.h"
#include "Core/QTETypes.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "UObject/StrongObjectPtr.h"

class UQTEComponent;
class UQTEDefinitionDataAsset;
class AActor;

/**
 * Coordinates an authority-driven QTE between the server (authority) and the owning
 * client (mirror), so both sides agree on input and on the final outcome. Handshake:
 *
 *  1. Start
 *  Authority calls Start(). For a remotely-controlled pawn, it stores the pending
 *  definition, RPCs Client_StartAuthorityQTE() to the owning client, and arms a
 *  "mirror ready" timeout.
 *
 *  2. Client mirror
 *  The client receives Client_StartAuthorityQTE() (HandleClientStart), starts its own
 *  local QTE mirror for responsive input, then RPCs ConfirmAuthorityQTEReady() back.
 *  
 *  3. Authority start
 *  The authority receives the confirmation (HandleServerConfirmReady), clears the
 *  timeout, and only now starts its own authoritative QTE.
 *  
 *  4. Input forwarding
 *  While running, the client forwards press/release input to the authority via reliable
 *  RPCs (Server_SubmitAuthorityPressedInput/Server_SubmitAuthorityReleasedInput),
 *  tagged with the current step index so the authority can reject stale/out-of-order
 *  input.
 *  
 *  5. Resolve
 *  When the authority's QTE finishes, Resolve() RPCs Client_CompleteAuthorityQTE()
 *  with the final result, and the client overwrites its local mirror outcome with it
 *  (authority always wins).
 *
 * If the client never confirms in time, or fails to start its mirror, the authority fails
 * the request (FailRequest) instead of running a QTE the client can't see/play.
 */
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
