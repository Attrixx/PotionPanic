#include "Components/QTEComponent.h"
#include "DrawDebugHelpers.h"
#include "InputAction.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "Core/QTESourceProvider.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(MS_QTEComponent, Log, All);

UQTEComponent::UQTEComponent()
	: AuthoritySession(*this)
	, InputBinder(*this)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

UQTEComponent::~UQTEComponent() = default;

void UQTEComponent::InitializeEnhancedInput(UEnhancedInputComponent* InEnhancedInputComponent)
{
	InputBinder.Initialize(InEnhancedInputComponent);

	if (IsQTERunning() && ShouldBindLocalInput())
	{
		InputBinder.BindForDefinition();
		InputBinder.AddMappingContext();
	}
}

int32 UQTEComponent::StartAuthorityQTE(UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor)
{
	return AuthoritySession.Start(InDefinition, InSourceActor);
}

void UQTEComponent::CancelAuthorityQTE(int32 RequestId)
{
	AuthoritySession.Cancel(RequestId);
}

bool UQTEComponent::StartQTEInternal(UQTEDefinitionDataAsset* InDefinition, UObject* InSourceObject, AActor* InInstigator)
{
	UE_LOG(MS_QTEComponent, Verbose, TEXT("StartQTE owner='%s' definition='%s' sourceObject='%s' instigator='%s'."),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(InDefinition),
		*GetNameSafe(InSourceObject),
		*GetNameSafe(InInstigator));

	if (!InDefinition || InDefinition->Steps.IsEmpty())
	{
		UE_LOG(MS_QTEComponent, Warning, TEXT("Cannot start QTE on '%s' with an invalid definition."), *GetNameSafe(GetOwner()));
		return false;
	}

	if (IsQTERunning())
	{
		InterruptQTE();
	}

	ActiveDefinition = InDefinition;
	UObject* ResolvedSourceObject = ResolveSourceObject(InSourceObject);
	UE_LOG(MS_QTEComponent, Verbose, TEXT("StartQTE resolvedSource='%s' qteType='%d' steps=%d."),
		*GetNameSafe(ResolvedSourceObject),
		static_cast<int32>(ActiveDefinition->GetQTEType()),
		ActiveDefinition->Steps.Num());

	SourceObject = ResolvedSourceObject;
	InstigatorActor = InInstigator ? InInstigator : Cast<AActor>(GetOwner());
	RuntimeState = FQTERuntimeState();
	RuntimeState.SourceObject = ResolvedSourceObject;
	RuntimeState.SourceActor = Cast<AActor>(ResolvedSourceObject);
	RuntimeState.Definition = ActiveDefinition;
	RuntimeState.QTEType = ActiveDefinition->GetQTEType();
	RuntimeState.Status = EQTEState::Running;
	RuntimeState.TotalSteps = ActiveDefinition->Steps.Num();
	RuntimeState.CurrentStepIndex = 0;
	LastResult = FQTEResult();
	HoldStartTime = 0.f;
	ActiveQTERuntime = FQTERuntime::Create(*this, *ActiveDefinition);
	if (!ActiveQTERuntime)
	{
		UE_LOG(MS_QTEComponent, Warning, TEXT("Cannot start QTE on '%s' because no runtime was created for definition '%s'."), *GetNameSafe(GetOwner()), *GetNameSafe(ActiveDefinition));
		ActiveDefinition = nullptr;
		SourceObject.Reset();
		InstigatorActor.Reset();
		return false;
	}

	if (ShouldBindLocalInput())
	{
		InputBinder.BindForDefinition();
		InputBinder.AddMappingContext();
	}
	SetComponentTickEnabled(true);
	if (ActiveQTERuntime)
	{
		ActiveQTERuntime->Start();
	}

	OnQTEStarted.Broadcast(RuntimeState);
	OnQTEStepChanged.Broadcast(RuntimeState);
	OnQTEUpdated.Broadcast(RuntimeState);
	UE_LOG(MS_QTEComponent, Verbose, TEXT("StartQTE broadcasted owner='%s' currentStep=%d action='%s' sourceActor='%s'."),
		*GetNameSafe(GetOwner()),
		RuntimeState.CurrentStepIndex,
		*GetNameSafe(GetCurrentStep() ? GetCurrentStep()->InputAction : nullptr),
		*GetNameSafe(RuntimeState.SourceActor));
	return true;
}

void UQTEComponent::CancelQTE()
{
	if (IsQTERunning())
	{
		FinishQTE(EQTEState::Canceled);
	}
}

void UQTEComponent::InterruptQTE()
{
	if (IsQTERunning())
	{
		FinishQTE(EQTEState::Interrupted);
	}
}

bool UQTEComponent::IsQTERunning() const
{
	return RuntimeState.Status == EQTEState::Running && ActiveDefinition != nullptr;
}

FQTERuntimeState UQTEComponent::GetCurrentQTEState() const
{
	return RuntimeState;
}

FQTEStepDefinition UQTEComponent::GetCurrentQTEStep() const
{
	const FQTEStepDefinition* CurrentStep = GetCurrentStep();
	return CurrentStep ? *CurrentStep : FQTEStepDefinition();
}

UQTEDefinitionDataAsset* UQTEComponent::GetCurrentQTEDefinition() const
{
	return ActiveDefinition;
}

UQTEDefinitionDataAsset* UQTEComponent::GetActiveDefinition() const
{
	return ActiveDefinition;
}

UInputMappingContext* UQTEComponent::GetInputMappingContext() const
{
	return MappingContext;
}

FQTEResult UQTEComponent::GetLastQTEResult() const
{
	return LastResult;
}

FQTERuntimeState& UQTEComponent::GetMutableRuntimeState()
{
	return RuntimeState;
}

const FQTERuntimeState& UQTEComponent::GetRuntimeState() const
{
	return RuntimeState;
}

float& UQTEComponent::GetMutableHoldStartTime()
{
	return HoldStartTime;
}

void UQTEComponent::BroadcastQTEStepChanged()
{
	OnQTEStepChanged.Broadcast(RuntimeState);
}

void UQTEComponent::BroadcastQTEUpdated()
{
	OnQTEUpdated.Broadcast(RuntimeState);
}

float UQTEComponent::GetAuthorityMirrorReadyTimeoutSeconds() const
{
	return AuthorityMirrorReadyTimeoutSeconds;
}

void UQTEComponent::NotifyClientStartAuthorityQTE(int32 RequestId, UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor)
{
	ClientStartAuthorityQTE(RequestId, InDefinition, InSourceActor);
}

void UQTEComponent::NotifyClientCancelAuthorityQTE(int32 RequestId)
{
	ClientCancelAuthorityQTE(RequestId);
}

void UQTEComponent::NotifyClientCompleteAuthorityQTE(int32 RequestId, const FQTEAuthorityResult& AuthorityResult)
{
	ClientCompleteAuthorityQTE(RequestId, AuthorityResult);
}

void UQTEComponent::NotifyServerConfirmAuthorityQTEReady(int32 RequestId)
{
	ServerConfirmAuthorityQTEReady(RequestId);
}

void UQTEComponent::NotifyServerAuthorityQTEStartFailed(int32 RequestId, const FText& FailureMessage)
{
	ServerNotifyAuthorityQTEStartFailed(RequestId, FailureMessage);
}

bool UQTEComponent::SubmitInputPressed(const UInputAction* InputAction)
{
	UE_LOG(MS_QTEComponent, VeryVerbose, TEXT("SubmitInputPressed owner='%s' action='%s' running=%d currentAction='%s' mash=%d/%d."),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(InputAction),
		IsQTERunning(),
		GetCurrentStep() && GetCurrentStep()->InputAction ? *GetNameSafe(GetCurrentStep()->InputAction) : TEXT("None"),
		RuntimeState.CurrentMashCount,
		RuntimeState.EffectiveMashTarget);

	if (!IsQTERunning() || !InputAction)
	{
		return false;
	}

	if (ShouldForwardAuthorityInput())
	{
		ServerSubmitAuthorityPressedInput(AuthoritySession.GetActiveRequestId(), RuntimeState.CurrentStepIndex);
	}

	const FQTEStepDefinition* CurrentStep = GetCurrentStep();
	if (!CurrentStep)
	{
		return false;
	}

	return ActiveQTERuntime ? ActiveQTERuntime->HandlePressed(InputAction) : false;
}

bool UQTEComponent::SubmitInputReleased(const UInputAction* InputAction)
{
	if (!IsQTERunning() || !InputAction)
	{
		return false;
	}

	if (ShouldForwardAuthorityInput())
	{
		ServerSubmitAuthorityReleasedInput(AuthoritySession.GetActiveRequestId(), RuntimeState.CurrentStepIndex);
	}

	if (!GetCurrentStep())
	{
		return false;
	}

	return ActiveQTERuntime ? ActiveQTERuntime->HandleReleased(InputAction) : false;
}

bool UQTEComponent::SubmitInputTriggered(const UInputAction* InputAction)
{
	if (!IsQTERunning() || !InputAction)
	{
		return false;
	}

	if (!GetCurrentStep())
	{
		return false;
	}

	return ActiveQTERuntime ? ActiveQTERuntime->HandleTriggered(InputAction) : false;
}

void UQTEComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AuthoritySession.HandleEndPlay();

	InputBinder.Unbind();
	InputBinder.RemoveMappingContext();
	SetComponentTickEnabled(false);
	Super::EndPlay(EndPlayReason);
}

void UQTEComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsQTERunning())
	{
		return;
	}

	RuntimeState.ElapsedTime += DeltaTime;
	RuntimeState.StepElapsedTime += DeltaTime;
	RefreshRuntimeState();

	if (!ShouldDeferFinishToAuthority() && ActiveQTERuntime && ActiveQTERuntime->TryAutoCompleteHoldStep())
	{
		if (!IsQTERunning())
		{
			return;
		}
	}

	if (!ShouldDeferFinishToAuthority() && RuntimeState.GlobalTimeRemaining <= 0.f && FQTEResolver::GetEffectiveGlobalTimeout(ActiveDefinition) > 0.f)
	{
		FinishQTE(EQTEState::Timeout);
		return;
	}

	if (!ShouldDeferFinishToAuthority() && RuntimeState.EffectiveStepTimeout > 0.f && RuntimeState.StepTimeRemaining <= 0.f)
	{
		FinishQTE(EQTEState::Timeout);
		return;
	}

	if (bDrawMashDebugFeedback)
	{
		DrawMashDebugFeedback();
	}
	OnQTEUpdated.Broadcast(RuntimeState);
}

const FQTEStepDefinition* UQTEComponent::GetCurrentStep() const
{
	return ActiveDefinition && ActiveDefinition->Steps.IsValidIndex(RuntimeState.CurrentStepIndex)
		? &ActiveDefinition->Steps[RuntimeState.CurrentStepIndex]
		: nullptr;
}

UObject* UQTEComponent::ResolveSourceObject(UObject* InSourceObject) const
{
	if (InSourceObject)
	{
		return InSourceObject;
	}

	AActor* OwnerActor = GetOwner();
	if (OwnerActor && OwnerActor->Implements<UQTESourceProvider>())
	{
		if (UObject* SourceObjectFromProvider = IQTESourceProvider::Execute_GetQTESourceObject(OwnerActor))
		{
			return SourceObjectFromProvider;
		}
	}

	return OwnerActor;
}

bool UQTEComponent::ShouldBindLocalInput() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	if (OwnerPawn->IsLocallyControlled())
	{
		return true;
	}

	return InputBinder.HasEnhancedInputComponent();
}

void UQTEComponent::DrawMashDebugFeedback() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	const FQTEStepDefinition* CurrentStep = GetCurrentStep();
	if (RuntimeState.Status != EQTEState::Running || !CurrentStep || CurrentStep->StepType != EQTEStepType::Mash)
	{
		return;
	}

	AActor* AnchorActor = RuntimeState.SourceActor ? RuntimeState.SourceActor.Get() : Cast<AActor>(SourceObject.Get());
	if (!AnchorActor)
	{
		AnchorActor = GetOwner();
	}

	if (!AnchorActor || !GetWorld())
	{
		return;
	}

	FVector Origin = FVector::ZeroVector;
	FVector Extent = FVector::ZeroVector;
	AnchorActor->GetActorBounds(true, Origin, Extent);

	const FVector Center = FVector(Origin.X, Origin.Y, Origin.Z + Extent.Z + MashDebugBarHeightOffset);
	const FVector Start = Center + FVector(0.f, -MashDebugBarWidth * 0.5f, 0.f);
	const FVector End = Center + FVector(0.f, MashDebugBarWidth * 0.5f, 0.f);
	const float MashPercent = RuntimeState.EffectiveMashTarget > 0
		? FMath::Clamp(static_cast<float>(RuntimeState.CurrentMashCount) / static_cast<float>(RuntimeState.EffectiveMashTarget), 0.f, 1.f)
		: 0.f;
	const FVector FillEnd = Start + (End - Start) * MashPercent;
	const FVector TextLocation = Center + FVector(0.f, 0.f, 18.f);
	const FString MashText = FString::Printf(TEXT("Mash %d/%d"), RuntimeState.CurrentMashCount, FMath::Max(RuntimeState.EffectiveMashTarget, 1));

	DrawDebugLine(GetWorld(), Start, End, FColor(40, 40, 40), false, 0.12f, 0, 8.f);
	DrawDebugLine(GetWorld(), Start, FillEnd, FColor(255, 64, 64), false, 0.12f, 0, 12.f);
	DrawDebugString(GetWorld(), TextLocation, MashText, nullptr, FColor::White, 0.f, true);
}

void UQTEComponent::FinishQTE(EQTEState FinalState, const FText& OverrideMessage)
{
	if (!ActiveDefinition)
	{
		return;
	}

	const bool bDeferFinishToAuthority = ShouldDeferFinishToAuthority();
	const FText ResolvedMessage = FQTEResolver::ResolveOutcomeMessage(ActiveDefinition, FinalState, OverrideMessage);

	if (bDeferFinishToAuthority)
	{
		RuntimeState.Status = FinalState;

		if (!ResolvedMessage.IsEmpty())
		{
			RuntimeState.FeedbackText = ResolvedMessage;
		}

		RefreshRuntimeState();
		SetComponentTickEnabled(false);
		InputBinder.Unbind();
		InputBinder.RemoveMappingContext();
		OnQTEUpdated.Broadcast(RuntimeState);
		return;
	}

	RuntimeState.Status = FinalState;

	if (!ResolvedMessage.IsEmpty())
	{
		RuntimeState.FeedbackText = ResolvedMessage;
	}

	RefreshRuntimeState();
	SetComponentTickEnabled(false);
	InputBinder.Unbind();
	InputBinder.RemoveMappingContext();

	LastResult = FQTEResolver::BuildResult(ActiveDefinition, SourceObject.Get(), InstigatorActor.Get(), RuntimeState, FinalState, OverrideMessage);
	UE_LOG(MS_QTEComponent, Verbose, TEXT("FinishQTE owner='%s' definition='%s' outcome='%d' completed=%d mistakes=%d elapsed=%.2f message='%s'."),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(ActiveDefinition),
		static_cast<int32>(FinalState),
		LastResult.CompletedStepCount,
		LastResult.Mistakes,
		LastResult.ElapsedTime,
		*LastResult.Message.ToString());
	ClearActiveSessionReferences();
	OnQTEUpdated.Broadcast(RuntimeState);
	OnQTEFinished.Broadcast(LastResult);
	AuthoritySession.Resolve(FQTEResolver::BuildAuthorityResult(ActiveDefinition, RuntimeState, LastResult, EQTEState::None));
}

void UQTEComponent::RefreshRuntimeState()
{
	if (ActiveQTERuntime)
	{
		ActiveQTERuntime->RefreshRuntimeState();
		return;
	}
}


bool UQTEComponent::ShouldForwardAuthorityInput() const
{
	return AuthoritySession.ShouldForwardInput();
}

bool UQTEComponent::ShouldDeferFinishToAuthority() const
{
	return AuthoritySession.ShouldDeferFinish();
}

void UQTEComponent::ApplyAuthorityCompletionToLocalMirror(const FQTEAuthorityResult& AuthorityResult)
{
	if (!ActiveDefinition)
	{
		return;
	}

	RuntimeState.Status = AuthorityResult.Outcome;
	RuntimeState.CompletedStepCount = AuthorityResult.CompletedStepCount;
	RuntimeState.Mistakes = AuthorityResult.Mistakes;
	RuntimeState.ElapsedTime = AuthorityResult.ElapsedTime;
	RuntimeState.FeedbackText = AuthorityResult.Message;
	RefreshRuntimeState();
	SetComponentTickEnabled(false);
	InputBinder.Unbind();
	InputBinder.RemoveMappingContext();

	LastResult = FQTEResolver::BuildResult(ActiveDefinition, SourceObject.Get(), InstigatorActor.Get(), RuntimeState, AuthorityResult.Outcome, AuthorityResult.Message);
	LastResult.Grade = AuthorityResult.Grade;
	LastResult.CompletedStepCount = AuthorityResult.CompletedStepCount;
	LastResult.FailedStepIndex = AuthorityResult.FailedStepIndex;
	LastResult.Mistakes = AuthorityResult.Mistakes;
	LastResult.ElapsedTime = AuthorityResult.ElapsedTime;
	LastResult.Message = AuthorityResult.Message;
	LastResult.FinalRuntimeState = RuntimeState;

	ClearActiveSessionReferences();
	OnQTEUpdated.Broadcast(RuntimeState);
	OnQTEFinished.Broadcast(LastResult);
}

void UQTEComponent::HandleEnhancedInputStarted(const FInputActionInstance& Instance)
{
	SubmitInputPressed(Instance.GetSourceAction().Get());
}

void UQTEComponent::HandleEnhancedInputTriggered(const FInputActionInstance& Instance)
{
	SubmitInputTriggered(Instance.GetSourceAction().Get());
}

void UQTEComponent::HandleEnhancedInputCompleted(const FInputActionInstance& Instance)
{
	SubmitInputReleased(Instance.GetSourceAction().Get());
}

void UQTEComponent::HandleEnhancedInputCanceled(const FInputActionInstance& Instance)
{
	SubmitInputReleased(Instance.GetSourceAction().Get());
}

void UQTEComponent::HandleAuthorityMirrorReadyTimeout()
{
	AuthoritySession.HandleMirrorReadyTimeout();
}

void UQTEComponent::ClearActiveSessionReferences()
{
	InputBinder.RemoveMappingContext();
	ActiveDefinition = nullptr;
	ActiveQTERuntime.Reset();
	SourceObject.Reset();
	InstigatorActor.Reset();
	HoldStartTime = 0.f;
	RuntimeState.bInputHeld = false;
}

void UQTEComponent::ClientStartAuthorityQTE_Implementation(int32 RequestId, UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor)
{
	AuthoritySession.HandleClientStart(RequestId, InDefinition, InSourceActor);
}

void UQTEComponent::ClientCancelAuthorityQTE_Implementation(int32 RequestId)
{
	AuthoritySession.HandleClientCancel(RequestId);
}

void UQTEComponent::ClientCompleteAuthorityQTE_Implementation(int32 RequestId, const FQTEAuthorityResult& AuthorityResult)
{
	AuthoritySession.HandleClientComplete(RequestId, AuthorityResult);
}

void UQTEComponent::ServerConfirmAuthorityQTEReady_Implementation(int32 RequestId)
{
	AuthoritySession.HandleServerConfirmReady(RequestId);
}

void UQTEComponent::ServerNotifyAuthorityQTEStartFailed_Implementation(int32 RequestId, const FText& FailureMessage)
{
	AuthoritySession.HandleServerStartFailed(RequestId, FailureMessage);
}

void UQTEComponent::ServerSubmitAuthorityPressedInput_Implementation(int32 RequestId, int32 StepIndex)
{
	AuthoritySession.HandleServerPressedInput(RequestId, StepIndex);
}

void UQTEComponent::ServerSubmitAuthorityReleasedInput_Implementation(int32 RequestId, int32 StepIndex)
{
	AuthoritySession.HandleServerReleasedInput(RequestId, StepIndex);
}
