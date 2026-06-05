#include "Replication/QTEAuthoritySession.h"

#include "InputAction.h"
#include "Components/QTEComponent.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(MS_QTEAuthoritySession, Log, All);

FQTEAuthoritySession::FQTEAuthoritySession() = default;

FQTEAuthoritySession::FQTEAuthoritySession(UQTEComponent& InOwnerComponent)
{
	Initialize(InOwnerComponent);
}

FQTEAuthoritySession::~FQTEAuthoritySession() = default;

void FQTEAuthoritySession::Initialize(UQTEComponent& InOwnerComponent)
{
	OwnerComponent = &InOwnerComponent;
}

UQTEComponent& FQTEAuthoritySession::GetOwnerComponent() const
{
	check(OwnerComponent);
	return *OwnerComponent;
}

int32 FQTEAuthoritySession::Start(UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor)
{
	AActor* OwnerActor = GetOwnerComponent().GetOwner();
	UE_LOG(MS_QTEAuthoritySession, Verbose, TEXT("QTEAuthority Start owner='%s' definition='%s' source='%s' authority=%d activeRequest=%d running=%d."),
		*GetNameSafe(OwnerActor),
		*GetNameSafe(InDefinition),
		*GetNameSafe(InSourceActor),
		OwnerActor ? OwnerActor->HasAuthority() : 0,
		ActiveRequestId,
		GetOwnerComponent().IsQTERunning());

	if (!OwnerActor || !OwnerActor->HasAuthority() || !InDefinition || ActiveRequestId != INDEX_NONE || GetOwnerComponent().IsQTERunning())
	{
		UE_LOG(MS_QTEAuthoritySession, Warning, TEXT("StartAuthorityQTE rejected on '%s'. Authority=%d Definition='%s' ActiveAuthorityRequestId=%d IsQTERunning=%d"),
			*GetNameSafe(OwnerActor),
			OwnerActor ? OwnerActor->HasAuthority() : 0,
			*GetNameSafe(InDefinition),
			ActiveRequestId,
			GetOwnerComponent().IsQTERunning());
		return INDEX_NONE;
	}

	const int32 RequestId = NextRequestId++;
	ActiveRequestId = RequestId;

	if (const APawn* OwnerPawn = Cast<APawn>(OwnerActor); OwnerPawn && !OwnerPawn->IsLocallyControlled())
	{
		PendingDefinition = InDefinition;
		PendingSourceActor = InSourceActor;
		StartMirrorReadyTimeout(RequestId);
		GetOwnerComponent().Client_StartAuthorityQTE(RequestId, InDefinition, InSourceActor);
		return RequestId;
	}

	if (!GetOwnerComponent().StartQTEInternal(InDefinition, InSourceActor, OwnerActor))
	{
		ActiveRequestId = INDEX_NONE;
		PendingDefinition = nullptr;
		PendingSourceActor.Reset();
		return INDEX_NONE;
	}

	PendingDefinition = nullptr;
	PendingSourceActor.Reset();
	return RequestId;
}

void FQTEAuthoritySession::Cancel(int32 RequestId)
{
	if (RequestId == INDEX_NONE || ActiveRequestId != RequestId)
	{
		return;
	}

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwnerComponent().GetOwner()); GetOwnerComponent().GetOwner() && GetOwnerComponent().GetOwner()->HasAuthority() && OwnerPawn && !OwnerPawn->IsLocallyControlled())
	{
		PendingDefinition = nullptr;
		PendingSourceActor.Reset();
		ClearMirrorReadyTimeout();
		GetOwnerComponent().Client_CancelAuthorityQTE(RequestId);
	}

	ActiveRequestId = INDEX_NONE;

	if (GetOwnerComponent().IsQTERunning())
	{
		GetOwnerComponent().CancelQTE();
	}
}

void FQTEAuthoritySession::HandleEndPlay()
{
	if (GetOwnerComponent().GetOwner() && GetOwnerComponent().GetOwner()->HasAuthority() && ActiveRequestId != INDEX_NONE)
	{
		FailRequest(ActiveRequestId, FText::FromString(TEXT("QTE interrupted because the owning actor ended play.")), true);
	}

	ClearMirrorReadyTimeout();
}

void FQTEAuthoritySession::HandleMirrorReadyTimeout()
{
	if (!GetOwnerComponent().GetOwner() || !GetOwnerComponent().GetOwner()->HasAuthority() || ActiveRequestId == INDEX_NONE || GetOwnerComponent().IsQTERunning() || !PendingDefinition)
	{
		return;
	}

	FailRequest(ActiveRequestId, FText::FromString(TEXT("Timed out waiting for client QTE mirror readiness.")), true);
}

void FQTEAuthoritySession::Resolve(const FQTEAuthorityResult& AuthorityResult)
{
	if (ActiveRequestId == INDEX_NONE)
	{
		return;
	}

	const int32 ResolvedRequestId = ActiveRequestId;
	if (GetOwnerComponent().GetOwner() && GetOwnerComponent().GetOwner()->HasAuthority())
	{
		ClearMirrorReadyTimeout();

		if (const APawn* OwnerPawn = Cast<APawn>(GetOwnerComponent().GetOwner()); OwnerPawn && !OwnerPawn->IsLocallyControlled())
		{
			GetOwnerComponent().Client_CompleteAuthorityQTE(ResolvedRequestId, AuthorityResult);
		}

		PendingDefinition = nullptr;
		PendingSourceActor.Reset();
		ActiveRequestId = INDEX_NONE;
		GetOwnerComponent().OnQTEAuthorityFinished.Broadcast(ResolvedRequestId, AuthorityResult);
	}
}

void FQTEAuthoritySession::HandleClientStart(int32 RequestId, UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor)
{
	UE_LOG(MS_QTEAuthoritySession, Verbose, TEXT("QTEAuthority ClientStart owner='%s' request=%d definition='%s' source='%s'."),
		*GetNameSafe(GetOwnerComponent().GetOwner()),
		RequestId,
		*GetNameSafe(InDefinition),
		*GetNameSafe(InSourceActor));

	if (GetOwnerComponent().IsQTERunning())
	{
		ActiveRequestId = INDEX_NONE;
		GetOwnerComponent().InterruptQTE();
	}

	ActiveRequestId = RequestId;

	if (!GetOwnerComponent().StartQTEInternal(InDefinition, InSourceActor, Cast<AActor>(GetOwnerComponent().GetOwner())))
	{
		GetOwnerComponent().Server_NotifyAuthorityQTEStartFailed(RequestId, FText::FromString(TEXT("Failed to start local mirror QTE.")));
		ActiveRequestId = INDEX_NONE;
		UE_LOG(MS_QTEAuthoritySession, Warning, TEXT("Failed to start local mirror QTE on '%s' for authority request '%d'."), *GetNameSafe(GetOwnerComponent().GetOwner()), RequestId);
		return;
	}

	GetOwnerComponent().Server_ConfirmAuthorityQTEReady(RequestId);
}

void FQTEAuthoritySession::HandleClientCancel(int32 RequestId)
{
	if (ActiveRequestId != RequestId)
	{
		return;
	}

	ActiveRequestId = INDEX_NONE;

	if (GetOwnerComponent().IsQTERunning())
	{
		GetOwnerComponent().CancelQTE();
	}
}

void FQTEAuthoritySession::HandleClientComplete(int32 RequestId, const FQTEAuthorityResult& AuthorityResult)
{
	if (ActiveRequestId != RequestId)
	{
		return;
	}

	ActiveRequestId = INDEX_NONE;

	const bool bWasRunning = GetOwnerComponent().IsQTERunning();
	const FQTEResult LastResult = GetOwnerComponent().GetLastQTEResult();
	const bool bResultDiffers =
		LastResult.Outcome != AuthorityResult.Outcome ||
		LastResult.Grade != AuthorityResult.Grade ||
		LastResult.CompletedStepCount != AuthorityResult.CompletedStepCount ||
		LastResult.FailedStepIndex != AuthorityResult.FailedStepIndex ||
		LastResult.Mistakes != AuthorityResult.Mistakes ||
		!LastResult.Message.EqualTo(AuthorityResult.Message);

	if (bWasRunning || bResultDiffers)
	{
		GetOwnerComponent().ApplyAuthorityCompletionToLocalMirror(AuthorityResult);
	}
}

void FQTEAuthoritySession::HandleServerConfirmReady(int32 RequestId)
{
	if (ActiveRequestId != RequestId || GetOwnerComponent().IsQTERunning() || !PendingDefinition)
	{
		return;
	}

	ClearMirrorReadyTimeout();

	UQTEDefinitionDataAsset* DefinitionToStart = PendingDefinition;
	AActor* SourceActorToStart = PendingSourceActor.Get();
	PendingDefinition = nullptr;
	PendingSourceActor.Reset();

	if (!GetOwnerComponent().StartQTEInternal(DefinitionToStart, SourceActorToStart, Cast<AActor>(GetOwnerComponent().GetOwner())))
	{
		FailRequest(RequestId, FText::FromString(TEXT("Failed to start authoritative QTE.")), true);
	}
}

void FQTEAuthoritySession::HandleServerStartFailed(int32 RequestId, const FText& FailureMessage)
{
	if (ActiveRequestId != RequestId || GetOwnerComponent().IsQTERunning() || !PendingDefinition)
	{
		return;
	}

	FailRequest(
		RequestId,
		!FailureMessage.IsEmpty() ? FailureMessage : FText::FromString(TEXT("Failed to start local mirror QTE.")),
		false);
}

void FQTEAuthoritySession::HandleServerPressedInput(int32 RequestId, int32 StepIndex)
{
	const FQTEStepDefinition* CurrentStep = GetOwnerComponent().GetCurrentStep();
	const UInputAction* ExpectedInputAction = CurrentStep ? CurrentStep->InputAction : nullptr;
	UE_LOG(MS_QTEAuthoritySession, VeryVerbose, TEXT("QTEAuthority ServerPressed owner='%s' request=%d activeRequest=%d clientStep=%d serverStep=%d expectedAction='%s' running=%d."),
		*GetNameSafe(GetOwnerComponent().GetOwner()),
		RequestId,
		ActiveRequestId,
		StepIndex,
		GetOwnerComponent().GetRuntimeState().CurrentStepIndex,
		*GetNameSafe(ExpectedInputAction),
		GetOwnerComponent().IsQTERunning());

	if (ActiveRequestId != RequestId || !GetOwnerComponent().IsQTERunning() || StepIndex != GetOwnerComponent().GetRuntimeState().CurrentStepIndex || !ExpectedInputAction)
	{
		return;
	}

	GetOwnerComponent().SubmitInputPressed(ExpectedInputAction);
}

void FQTEAuthoritySession::HandleServerReleasedInput(int32 RequestId, int32 StepIndex)
{
	const FQTEStepDefinition* CurrentStep = GetOwnerComponent().GetCurrentStep();
	const UInputAction* ExpectedInputAction = CurrentStep ? CurrentStep->InputAction : nullptr;
	UE_LOG(MS_QTEAuthoritySession, VeryVerbose, TEXT("QTEAuthority ServerReleased owner='%s' request=%d activeRequest=%d clientStep=%d serverStep=%d expectedAction='%s' running=%d."),
		*GetNameSafe(GetOwnerComponent().GetOwner()),
		RequestId,
		ActiveRequestId,
		StepIndex,
		GetOwnerComponent().GetRuntimeState().CurrentStepIndex,
		*GetNameSafe(ExpectedInputAction),
		GetOwnerComponent().IsQTERunning());

	if (ActiveRequestId != RequestId || !GetOwnerComponent().IsQTERunning() || StepIndex != GetOwnerComponent().GetRuntimeState().CurrentStepIndex || !ExpectedInputAction)
	{
		return;
	}

	GetOwnerComponent().SubmitInputReleased(ExpectedInputAction);
}

bool FQTEAuthoritySession::ShouldForwardInput() const
{
	return GetOwnerComponent().IsQTERunning() && ActiveRequestId != INDEX_NONE && GetOwnerComponent().GetOwner() && !GetOwnerComponent().GetOwner()->HasAuthority();
}

bool FQTEAuthoritySession::ShouldDeferFinish() const
{
	return ActiveRequestId != INDEX_NONE && GetOwnerComponent().GetOwner() && !GetOwnerComponent().GetOwner()->HasAuthority();
}

int32 FQTEAuthoritySession::GetActiveRequestId() const
{
	return ActiveRequestId;
}

void FQTEAuthoritySession::StartMirrorReadyTimeout(int32 RequestId)
{
	if (!GetOwnerComponent().GetOwner() || !GetOwnerComponent().GetOwner()->HasAuthority() || ActiveRequestId != RequestId)
	{
		return;
	}

	if (UWorld* World = GetOwnerComponent().GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MirrorReadyTimeoutHandle,
			OwnerComponent, // We can't bind to ourselves (binding needs a UObject ptr)
			&UQTEComponent::HandleAuthorityMirrorReadyTimeout,
			GetOwnerComponent().GetAuthorityMirrorReadyTimeoutSeconds(),
			false);
	}
}

void FQTEAuthoritySession::ClearMirrorReadyTimeout()
{
	if (UWorld* World = GetOwnerComponent().GetWorld())
	{
		World->GetTimerManager().ClearTimer(MirrorReadyTimeoutHandle);
	}
}

void FQTEAuthoritySession::FailRequest(int32 RequestId, const FText& FailureMessage, bool bNotifyOwningClient)
{
	if (!GetOwnerComponent().GetOwner() || !GetOwnerComponent().GetOwner()->HasAuthority() || ActiveRequestId != RequestId)
	{
		return;
	}

	ClearMirrorReadyTimeout();
	PendingDefinition = nullptr;
	PendingSourceActor.Reset();

	FQTEAuthorityResult FailedResult;
	FailedResult.Outcome = EQTEState::Failure;
	FailedResult.Grade = EQTEGrade::Fail;
	FailedResult.Message = FailureMessage;

	ActiveRequestId = INDEX_NONE;

	if (bNotifyOwningClient)
	{
		if (const APawn* OwnerPawn = Cast<APawn>(GetOwnerComponent().GetOwner()); OwnerPawn && !OwnerPawn->IsLocallyControlled())
		{
			GetOwnerComponent().Client_CompleteAuthorityQTE(RequestId, FailedResult);
		}
	}

	GetOwnerComponent().OnQTEAuthorityFinished.Broadcast(RequestId, FailedResult);
}
