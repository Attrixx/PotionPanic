#include "Components/QTEComponent.h"

#include "DrawDebugHelpers.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "Core/QTESourceProvider.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(MS_QTEComponent, Log, All);

// ============================================================
//  Lifecycle
// ============================================================

UQTEComponent::UQTEComponent()
	: AuthoritySession(*this)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

UQTEComponent::~UQTEComponent() = default;

void UQTEComponent::SetupInputComponent_Implementation(UEnhancedInputComponent* EIC)
{
	IInputBindable::SetupInputComponent_Implementation(EIC);
	EnhancedInputComponent = EIC;

	if (IsQTERunning() && ShouldBindLocalInput())
	{
		BindInputForDefinition();
		AddInputMappingContext();
	}
}

void UQTEComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AuthoritySession.HandleEndPlay();
	UnbindInput();
	RemoveInputMappingContext();
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

	if (!ShouldDeferFinishToAuthority())
	{
		if (ActiveQTERuntime && ActiveQTERuntime->TryAutoCompleteHoldStep() && !IsQTERunning())
		{
			return;
		}

		if (RuntimeState.GlobalTimeRemaining <= 0.f && ActiveDefinition->GetEffectiveGlobalTimeout() > 0.f)
		{
			FinishQTE(EQTEState::Timeout);
			return;
		}

		if (RuntimeState.EffectiveStepTimeout > 0.f && RuntimeState.StepTimeRemaining <= 0.f)
		{
			FinishQTE(EQTEState::Timeout);
			return;
		}
	}

	if (bDrawMashDebugFeedback)
	{
		DrawMashDebugFeedback();
	}
	OnQTEUpdated.Broadcast(RuntimeState);
}

// ============================================================
//  Authority API
// ============================================================

int32 UQTEComponent::StartAuthorityQTE(UQTEDefinitionDataAsset* InDefinition, AActor* InSourceActor)
{
	return AuthoritySession.Start(InDefinition, InSourceActor);
}

void UQTEComponent::CancelAuthorityQTE(int32 RequestId)
{
	AuthoritySession.Cancel(RequestId);
}

// ============================================================
//  QTE Control
// ============================================================

bool UQTEComponent::StartQTEInternal(UQTEDefinitionDataAsset* InDefinition, UObject* InSourceObject, AActor* InInstigator)
{
	UE_LOG(MS_QTEComponent,
		Verbose,
		TEXT("StartQTE owner='%s' definition='%s' sourceObject='%s' instigator='%s'."),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(InDefinition),
		*GetNameSafe(InSourceObject),
		*GetNameSafe(InInstigator));

	if (!InDefinition || InDefinition->Steps.IsEmpty())
	{
		UE_LOG(MS_QTEComponent, Warning, TEXT("Cannot start QTE on '%s': invalid definition."), *GetNameSafe(GetOwner()));
		return false;
	}

	if (IsQTERunning())
	{
		InterruptQTE();
	}

	ActiveDefinition = InDefinition;
	UObject* ResolvedSource = ResolveSourceObject(InSourceObject);

	SourceObject = ResolvedSource;
	InstigatorActor = InInstigator ? InInstigator : Cast<AActor>(GetOwner());

	RuntimeState = FQTERuntimeState();
	RuntimeState.SourceObject = ResolvedSource;
	RuntimeState.SourceActor = Cast<AActor>(ResolvedSource);
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
		UE_LOG(MS_QTEComponent,
			Warning,
			TEXT("Cannot start QTE on '%s': no runtime for definition '%s'."),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(ActiveDefinition));
		ActiveDefinition = nullptr;
		SourceObject.Reset();
		InstigatorActor.Reset();
		return false;
	}

	if (ShouldBindLocalInput())
	{
		BindInputForDefinition();
		AddInputMappingContext();
	}

	SetComponentTickEnabled(true);
	ActiveQTERuntime->Start();

	OnQTEStarted.Broadcast(RuntimeState);
	OnQTEStepChanged.Broadcast(RuntimeState);
	OnQTEUpdated.Broadcast(RuntimeState);
	return true;
}

void UQTEComponent::CancelQTE()
{
	if (IsQTERunning()) FinishQTE(EQTEState::Canceled);
}

void UQTEComponent::InterruptQTE()
{
	if (IsQTERunning()) FinishQTE(EQTEState::Interrupted);
}

void UQTEComponent::FinishQTE(EQTEState FinalState, const FText& OverrideMessage)
{
	if (!ActiveDefinition) return;

	const FText ResolvedMessage = ActiveDefinition->ResolveOutcomeMessage(FinalState, OverrideMessage);

	RuntimeState.Status = FinalState;
	if (!ResolvedMessage.IsEmpty())
	{
		RuntimeState.FeedbackText = ResolvedMessage;
	}

	RefreshRuntimeState();
	SetComponentTickEnabled(false);
	UnbindInput();
	RemoveInputMappingContext();

	if (ShouldDeferFinishToAuthority())
	{
		OnQTEUpdated.Broadcast(RuntimeState);
		return;
	}

	LastResult = ActiveDefinition->BuildResult(SourceObject.Get(), InstigatorActor.Get(), RuntimeState, FinalState, OverrideMessage);
	UE_LOG(MS_QTEComponent,
		Verbose,
		TEXT("FinishQTE owner='%s' definition='%s' outcome=%d completed=%d mistakes=%d elapsed=%.2f message='%s'."),
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
	AuthoritySession.Resolve(ActiveDefinition->BuildAuthorityResult(RuntimeState, LastResult, EQTEState::None));
}

// ============================================================
//  Input Submission
// ============================================================

bool UQTEComponent::SubmitInputPressed(const UInputAction* InputAction)
{
	UE_LOG(MS_QTEComponent,
		VeryVerbose,
		TEXT("SubmitInputPressed owner='%s' action='%s' running=%d mash=%d/%d."),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(InputAction),
		IsQTERunning(),
		RuntimeState.CurrentMashCount,
		RuntimeState.EffectiveMashTarget);

	if (!IsQTERunning() || !InputAction) return false;

	if (ShouldForwardAuthorityInput())
		Server_SubmitAuthorityPressedInput(AuthoritySession.GetActiveRequestId(), RuntimeState.CurrentStepIndex);

	if (!GetCurrentStep()) return false;
	return ActiveQTERuntime ? ActiveQTERuntime->HandlePressed(InputAction) : false;
}

bool UQTEComponent::SubmitInputReleased(const UInputAction* InputAction)
{
	if (!IsQTERunning() || !InputAction) return false;

	if (ShouldForwardAuthorityInput())
		Server_SubmitAuthorityReleasedInput(AuthoritySession.GetActiveRequestId(), RuntimeState.CurrentStepIndex);

	if (!GetCurrentStep()) return false;
	return ActiveQTERuntime ? ActiveQTERuntime->HandleReleased(InputAction) : false;
}

bool UQTEComponent::SubmitInputTriggered(const UInputAction* InputAction)
{
	if (!IsQTERunning() || !InputAction || !GetCurrentStep()) return false;
	return ActiveQTERuntime ? ActiveQTERuntime->HandleTriggered(InputAction) : false;
}

// ============================================================
//  Input Binding (merged from FQTEInputBinder)
// ============================================================

void UQTEComponent::BindInputForDefinition()
{
	UnbindInput();

	if (!EnhancedInputComponent.IsValid() || !ActiveDefinition)
	{
		UE_LOG(MS_QTEComponent,
			Warning,
			TEXT("Cannot bind QTE input on '%s'. EICValid=%d Definition='%s'"),
			*GetNameSafe(GetOwner()),
			EnhancedInputComponent.IsValid(),
			*GetNameSafe(ActiveDefinition));
		return;
	}

	TSet<TObjectPtr<UInputAction>> UniqueActions;

	auto BindActionOnce = [this, &UniqueActions](const UInputAction* Action)
	{
		UInputAction* Mutable = const_cast<UInputAction*>(Action);
		if (!Mutable || UniqueActions.Contains(Mutable)) return;
		UniqueActions.Add(Mutable);

		FBoundInputHandles Handles{
			.StartedHandle = EnhancedInputComponent->BindAction(Action, ETriggerEvent::Started, this, &ThisClass::HandleEnhancedInputStarted).GetHandle(),
			.TriggeredHandle = EnhancedInputComponent->BindAction(Action, ETriggerEvent::Triggered, this, &ThisClass::HandleEnhancedInputTriggered).GetHandle(),
			.CompletedHandle = EnhancedInputComponent->BindAction(Action, ETriggerEvent::Completed, this, &ThisClass::HandleEnhancedInputCompleted).GetHandle(),
			.CanceledHandle = EnhancedInputComponent->BindAction(Action, ETriggerEvent::Canceled, this, &ThisClass::HandleEnhancedInputCanceled).GetHandle(),
		};
		BoundInputHandles.Add(Mutable, Handles);
	};

	if (MappingContext)
	{
		for (const FEnhancedActionKeyMapping& Mapping : MappingContext->GetMappings())
		{
			BindActionOnce(Mapping.Action.Get());
		}
	}

	for (const FQTEStepDefinition& Step : ActiveDefinition->Steps)
	{
		BindActionOnce(Step.InputAction);
	}
}

void UQTEComponent::UnbindInput()
{
	if (!EnhancedInputComponent.IsValid())
	{
		BoundInputHandles.Reset();
		return;
	}

	for (const TPair<TObjectPtr<UInputAction>, FBoundInputHandles>& Pair : BoundInputHandles)
	{
		EnhancedInputComponent->RemoveBindingByHandle(Pair.Value.StartedHandle);
		EnhancedInputComponent->RemoveBindingByHandle(Pair.Value.TriggeredHandle);
		EnhancedInputComponent->RemoveBindingByHandle(Pair.Value.CompletedHandle);
		EnhancedInputComponent->RemoveBindingByHandle(Pair.Value.CanceledHandle);
	}
	BoundInputHandles.Reset();
}

void UQTEComponent::AddInputMappingContext()
{
	if (bAddedMappingContext) return;

	if (!MappingContext)
	{
		UE_LOG(MS_QTEComponent, Warning, TEXT("Cannot add QTE mapping context on '%s': MappingContext not set."), *GetNameSafe(GetOwner()));
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Sub = GetEnhancedInputSubsystem())
	{
		Sub->AddMappingContext(MappingContext, 100);
		bAddedMappingContext = true;
	}
	else
	{
		UE_LOG(MS_QTEComponent, Warning, TEXT("Cannot add QTE mapping context on '%s': no input subsystem available."), *GetNameSafe(GetOwner()));
	}
}

void UQTEComponent::RemoveInputMappingContext()
{
	if (!bAddedMappingContext || !MappingContext)
	{
		bAddedMappingContext = false;
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Sub = GetEnhancedInputSubsystem())
	{
		Sub->RemoveMappingContext(MappingContext);
	}
	bAddedMappingContext = false;
}

bool UQTEComponent::HasEnhancedInputComponent() const
{
	return EnhancedInputComponent.IsValid();
}

UEnhancedInputLocalPlayerSubsystem* UQTEComponent::GetEnhancedInputSubsystem() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return nullptr;

	const APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return nullptr;

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP) return nullptr;

	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}

// ============================================================
//  Accessors / Helpers
// ============================================================

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
	const auto* S = GetCurrentStep();
	return S ? *S : FQTEStepDefinition();
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

float UQTEComponent::GetAuthorityMirrorReadyTimeoutSeconds() const
{
	return AuthorityMirrorReadyTimeoutSeconds;
}

void UQTEComponent::HandleAuthorityMirrorReadyTimeout()
{
	AuthoritySession.HandleMirrorReadyTimeout();
}

void UQTEComponent::BroadcastQTEStepChanged()
{
	OnQTEStepChanged.Broadcast(RuntimeState);
}

void UQTEComponent::BroadcastQTEUpdated()
{
	OnQTEUpdated.Broadcast(RuntimeState);
}

const FQTEStepDefinition* UQTEComponent::GetCurrentStep() const
{
	return ActiveDefinition && ActiveDefinition->Steps.IsValidIndex(RuntimeState.CurrentStepIndex)
		? &ActiveDefinition->Steps[RuntimeState.CurrentStepIndex]
		: nullptr;
}

void UQTEComponent::RefreshRuntimeState()
{
	if (ActiveQTERuntime) ActiveQTERuntime->RefreshRuntimeState();
}

bool UQTEComponent::ShouldForwardAuthorityInput() const
{
	return AuthoritySession.ShouldForwardInput();
}

bool UQTEComponent::ShouldDeferFinishToAuthority() const
{
	return AuthoritySession.ShouldDeferFinish();
}

bool UQTEComponent::ShouldBindLocalInput() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn) return false;
	return Pawn->IsLocallyControlled() || HasEnhancedInputComponent();
}

UObject* UQTEComponent::ResolveSourceObject(UObject* InSourceObject) const
{
	if (InSourceObject) return InSourceObject;

	AActor* Owner = GetOwner();
	if (Owner && Owner->Implements<UQTESourceProvider>())
	{
		if (UObject* FromProvider = IQTESourceProvider::Execute_GetQTESourceObject(Owner))
			return FromProvider;
	}
	return Owner;
}

void UQTEComponent::ClearActiveSessionReferences()
{
	RemoveInputMappingContext();
	ActiveDefinition = nullptr;
	ActiveQTERuntime.Reset();
	SourceObject.Reset();
	InstigatorActor.Reset();
	HoldStartTime = 0.f;
	RuntimeState.bInputHeld = false;
}

void UQTEComponent::ApplyAuthorityCompletionToLocalMirror(const FQTEAuthorityResult& AuthorityResult)
{
	if (!ActiveDefinition) return;

	RuntimeState.Status = AuthorityResult.Outcome;
	RuntimeState.CompletedStepCount = AuthorityResult.CompletedStepCount;
	RuntimeState.Mistakes = AuthorityResult.Mistakes;
	RuntimeState.ElapsedTime = AuthorityResult.ElapsedTime;
	RuntimeState.FeedbackText = AuthorityResult.Message;

	RefreshRuntimeState();
	SetComponentTickEnabled(false);
	UnbindInput();
	RemoveInputMappingContext();

	LastResult = ActiveDefinition->BuildResult(
		SourceObject.Get(),
		InstigatorActor.Get(),
		RuntimeState,
		AuthorityResult.Outcome,
		AuthorityResult.Message);
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

// ============================================================
//  Enhanced Input Handlers
// ============================================================

void UQTEComponent::HandleEnhancedInputStarted(const FInputActionInstance& I)
{
	SubmitInputPressed(I.GetSourceAction().Get());
}

void UQTEComponent::HandleEnhancedInputTriggered(const FInputActionInstance& I)
{
	SubmitInputTriggered(I.GetSourceAction().Get());
}

void UQTEComponent::HandleEnhancedInputCompleted(const FInputActionInstance& I)
{
	SubmitInputReleased(I.GetSourceAction().Get());
}

void UQTEComponent::HandleEnhancedInputCanceled(const FInputActionInstance& I)
{
	SubmitInputReleased(I.GetSourceAction().Get());
}

void UQTEComponent::DrawMashDebugFeedback() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn || !Pawn->IsLocallyControlled()) return;

	const FQTEStepDefinition* Step = GetCurrentStep();
	if (RuntimeState.Status != EQTEState::Running || !Step || Step->StepType != EQTEStepType::Mash) return;

	AActor* Anchor = RuntimeState.SourceActor ? RuntimeState.SourceActor.Get() : Cast<AActor>(SourceObject.Get());
	if (!Anchor) Anchor = GetOwner();
	if (!Anchor || !GetWorld()) return;

	FVector Origin, Extent;
	Anchor->GetActorBounds(true, Origin, Extent);

	const FVector Center = FVector(Origin.X, Origin.Y, Origin.Z + Extent.Z + MashDebugBarHeightOffset);
	const FVector BarStart = Center + FVector(0.f, -MashDebugBarWidth * 0.5f, 0.f);
	const FVector BarEnd = Center + FVector(0.f, MashDebugBarWidth * 0.5f, 0.f);
	const float Pct = RuntimeState.EffectiveMashTarget > 0
		? FMath::Clamp(static_cast<float>(RuntimeState.CurrentMashCount) / static_cast<float>(RuntimeState.EffectiveMashTarget), 0.f, 1.f)
		: 0.f;

	DrawDebugLine(GetWorld(), BarStart, BarEnd, FColor(40, 40, 40), false, 0.12f, 0, 8.f);
	DrawDebugLine(GetWorld(), BarStart, BarStart + (BarEnd - BarStart) * Pct, FColor(255, 64, 64), false, 0.12f, 0, 12.f);
	DrawDebugString(GetWorld(),
		Center + FVector(0.f, 0.f, 18.f),
		FString::Printf(TEXT("Mash %d/%d"), RuntimeState.CurrentMashCount, FMath::Max(RuntimeState.EffectiveMashTarget, 1)),
		nullptr,
		FColor::White,
		0.f,
		true);
}

// ============================================================
//  RPC Implementations
// ============================================================

void UQTEComponent::Client_StartAuthorityQTE_Implementation(int32 Id, UQTEDefinitionDataAsset* Def, AActor* Src)
{
	AuthoritySession.HandleClientStart(Id, Def, Src);
}

void UQTEComponent::Client_CancelAuthorityQTE_Implementation(int32 Id)
{
	AuthoritySession.HandleClientCancel(Id);
}

void UQTEComponent::Client_CompleteAuthorityQTE_Implementation(int32 Id, const FQTEAuthorityResult& R)
{
	AuthoritySession.HandleClientComplete(Id, R);
}

void UQTEComponent::Server_ConfirmAuthorityQTEReady_Implementation(int32 Id)
{
	AuthoritySession.HandleServerConfirmReady(Id);
}

void UQTEComponent::Server_NotifyAuthorityQTEStartFailed_Implementation(int32 Id, const FText& Msg)
{
	AuthoritySession.HandleServerStartFailed(Id, Msg);
}

void UQTEComponent::Server_SubmitAuthorityPressedInput_Implementation(int32 Id, int32 Step)
{
	AuthoritySession.HandleServerPressedInput(Id, Step);
}

void UQTEComponent::Server_SubmitAuthorityReleasedInput_Implementation(int32 Id, int32 Step)
{
	AuthoritySession.HandleServerReleasedInput(Id, Step);
}
