#include "Input/QTEInputBinder.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Components/QTEComponent.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(MS_QTEInputBinder, Log, All);

FQTEInputBinder::FQTEInputBinder() = default;

FQTEInputBinder::FQTEInputBinder(UQTEComponent& InOwnerComponent)
{
	InitializeOwner(InOwnerComponent);
}

FQTEInputBinder::~FQTEInputBinder() = default;

void FQTEInputBinder::InitializeOwner(UQTEComponent& InOwnerComponent)
{
	OwnerComponent = &InOwnerComponent;
}

UQTEComponent& FQTEInputBinder::GetOwnerComponent() const
{
	check(OwnerComponent);
	return *OwnerComponent;
}

void FQTEInputBinder::Initialize(UEnhancedInputComponent* InEnhancedInputComponent)
{
	EnhancedInputComponent = InEnhancedInputComponent;
}

void FQTEInputBinder::BindForDefinition()
{
	Unbind();

	if (!EnhancedInputComponent.IsValid() || !GetOwnerComponent().GetActiveDefinition())
	{
		UE_LOG(MS_QTEInputBinder, Warning, TEXT("Cannot bind QTE input on '%s'. EnhancedInputComponentValid=%d Definition='%s'"),
			*GetNameSafe(GetOwnerComponent().GetOwner()),
			EnhancedInputComponent.IsValid(),
			*GetNameSafe(GetOwnerComponent().GetActiveDefinition()));
		return;
	}

	auto BindActionOnce = [this](const UInputAction* InputAction, TSet<TObjectPtr<UInputAction>>& UniqueActions)
	{
		UInputAction* MutableInputAction = const_cast<UInputAction*>(InputAction);
		if (!MutableInputAction || UniqueActions.Contains(MutableInputAction))
		{
			return;
		}

		UniqueActions.Add(MutableInputAction);

		FBoundInputHandles Handles;
		Handles.StartedHandle = EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Started, &GetOwnerComponent(), &UQTEComponent::HandleEnhancedInputStarted).GetHandle();
		Handles.TriggeredHandle = EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Triggered, &GetOwnerComponent(), &UQTEComponent::HandleEnhancedInputTriggered).GetHandle();
		Handles.CompletedHandle = EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Completed, &GetOwnerComponent(), &UQTEComponent::HandleEnhancedInputCompleted).GetHandle();
		Handles.CanceledHandle = EnhancedInputComponent->BindAction(InputAction, ETriggerEvent::Canceled, &GetOwnerComponent(), &UQTEComponent::HandleEnhancedInputCanceled).GetHandle();
		BoundInputHandles.Add(MutableInputAction, Handles);
	};

	TSet<TObjectPtr<UInputAction>> UniqueActions;
	if (GetOwnerComponent().GetInputMappingContext())
	{
		for (const FEnhancedActionKeyMapping& Mapping : GetOwnerComponent().GetInputMappingContext()->GetMappings())
		{
			BindActionOnce(Mapping.Action.Get(), UniqueActions);
		}
	}

	for (const FQTEStepDefinition& Step : GetOwnerComponent().GetActiveDefinition()->Steps)
	{
		BindActionOnce(Step.InputAction, UniqueActions);
	}
}

void FQTEInputBinder::Unbind()
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

void FQTEInputBinder::AddMappingContext()
{
	if (bAddedMappingContext)
	{
		return;
	}

	if (!GetOwnerComponent().GetInputMappingContext())
	{
		UE_LOG(MS_QTEInputBinder, Warning, TEXT("Cannot add QTE mapping context on '%s' because QTEComponent.MappingContext is not set."),
			*GetNameSafe(GetOwnerComponent().GetOwner()));
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwnerComponent().GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController)
	{
		UE_LOG(MS_QTEInputBinder, Warning, TEXT("Cannot add QTE mapping context on '%s' because no PlayerController is available."), *GetNameSafe(GetOwnerComponent().GetOwner()));
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		UE_LOG(MS_QTEInputBinder, Warning, TEXT("Cannot add QTE mapping context on '%s' because no LocalPlayer is available."), *GetNameSafe(GetOwnerComponent().GetOwner()));
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->AddMappingContext(GetOwnerComponent().GetInputMappingContext(), 100);
		bAddedMappingContext = true;
	}
}

void FQTEInputBinder::RemoveMappingContext()
{
	if (!bAddedMappingContext || !GetOwnerComponent().GetInputMappingContext())
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwnerComponent().GetOwner());
	if (!OwnerPawn)
	{
		bAddedMappingContext = false;
		return;
	}

	APlayerController* PlayerController = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PlayerController)
	{
		bAddedMappingContext = false;
		return;
	}

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		bAddedMappingContext = false;
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		Subsystem->RemoveMappingContext(GetOwnerComponent().GetInputMappingContext());
	}

	bAddedMappingContext = false;
}

bool FQTEInputBinder::HasEnhancedInputComponent() const
{
	return EnhancedInputComponent.IsValid();
}
