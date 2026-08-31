#include "Activities/QTEActivityStep.h"
#include "ActivityExecutor.h"
#include "Misc/DataValidation.h"
#include "Components/QTEComponent.h"
#include "Core/QTEDefinitionDataAsset.h"
#include "Widgets/QTEActivityDisplay.h"

DEFINE_LOG_CATEGORY_STATIC(MS_QTEActivityStep, Log, All);

#if WITH_EDITOR
EDataValidationResult UQTEActivityStepSetting::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!QTEDefinition)
	{
		Context.AddError(FText::FromString("QTEDefinition must be assigned."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

UActivityStep* UQTEActivityStepSetting::CreateStep_Implementation(UObject* Outer) const
{
	UQTEActivityStep* Step = NewObject<UQTEActivityStep>(Outer);
	Step->QTEDefinition = QTEDefinition;
	Step->QTEWidgetClass = QTEWidgetClass;
	return Step;
}

void UQTEActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
	UE_LOG(MS_QTEActivityStep, Log, TEXT("StartStep step='%s' definition='%s' lastInstigator='%s'."),
		*GetNameSafe(this),
		*GetNameSafe(QTEDefinition),
		*GetNameSafe(LastInstigator));

	StepResult = FActivityStepResult();
	ActiveRequestId = INDEX_NONE;
	bWaitingForInstigator = false;
	HideQTEActivityDisplay();
	UnbindAuthorityDelegate();

	if (!QTEDefinition)
	{
		FQTEAuthorityResult AuthorityResult;
		AuthorityResult.Outcome = EQTEState::Failure;
		AuthorityResult.Message = FText::FromString(TEXT("QTE activity step has no QTE definition."));
		FinishFromResult(AuthorityResult);
		return;
	}

	if (LastInstigator)
	{
		UE_LOG(MS_QTEActivityStep, Log, TEXT("StartStep immediately starts QTE with instigator='%s'."), *GetNameSafe(LastInstigator));
		TryStartQTE(LastInstigator);
		return;
	}

	UE_LOG(MS_QTEActivityStep, Log, TEXT("StartStep waiting for instigator interaction."));
	bWaitingForInstigator = true;
}

void UQTEActivityStep::OnInteract_Implementation(AActor* Instigator)
{
	UE_LOG(MS_QTEActivityStep, Log, TEXT("OnInteract step='%s' waiting=%d activeRequest=%d instigator='%s'."),
		*GetNameSafe(this),
		bWaitingForInstigator,
		ActiveRequestId,
		*GetNameSafe(Instigator));

	if (!bWaitingForInstigator || ActiveRequestId != INDEX_NONE || !Instigator)
	{
		return;
	}

	TryStartQTE(Instigator);
}

void UQTEActivityStep::CancelStep_Implementation()
{
	bWaitingForInstigator = false;
	HideQTEActivityDisplay();

	if (ActiveQTEComponent.IsValid() && ActiveRequestId != INDEX_NONE)
	{
		ActiveQTEComponent->CancelAuthorityQTE(ActiveRequestId);
	}

	UnbindAuthorityDelegate();
}

bool UQTEActivityStep::TryStartQTE(AActor* Instigator)
{
	UE_LOG(MS_QTEActivityStep, Log, TEXT("TryStartQTE definition='%s' instigator='%s'."),
		*GetNameSafe(QTEDefinition),
		*GetNameSafe(Instigator));

	if (!Instigator || !QTEDefinition)
	{
		return false;
	}

	UQTEComponent* QTEComponent = Instigator->FindComponentByClass<UQTEComponent>();
	if (!QTEComponent)
	{
		UE_LOG(MS_QTEActivityStep, Warning, TEXT("Actor '%s' cannot run QTE activity step because it has no QTEComponent."), *GetNameSafe(Instigator));
		FQTEAuthorityResult AuthorityResult;
		AuthorityResult.Outcome = EQTEState::Failure;
		AuthorityResult.Message = FText::FromString(TEXT("Instigator has no QTE component."));
		FinishFromResult(AuthorityResult);
		return false;
	}

	UActivityExecutor* ActivityExecutor = GetTypedOuter<UActivityExecutor>();
	AActor* SourceActor = ActivityExecutor ? ActivityExecutor->GetOwner() : nullptr;
	UE_LOG(MS_QTEActivityStep, Log, TEXT("TryStartQTE qteComponent='%s' sourceActor='%s' executor='%s'."),
		*GetNameSafe(QTEComponent),
		*GetNameSafe(SourceActor),
		*GetNameSafe(ActivityExecutor));

	ActiveQTEComponent = QTEComponent;
	ActiveQTEComponent->OnQTEAuthorityFinished.AddUObject(this, &UQTEActivityStep::HandleAuthorityQTEFinished);
	ActiveRequestId = ActiveQTEComponent->StartAuthorityQTE(QTEDefinition, SourceActor);

	if (ActiveRequestId == INDEX_NONE)
	{
		UE_LOG(MS_QTEActivityStep, Warning, TEXT("Actor '%s' failed to start an authority-driven QTE for activity step '%s'."), *GetNameSafe(Instigator), *GetNameSafe(QTEDefinition));
		FQTEAuthorityResult AuthorityResult;
		AuthorityResult.Outcome = EQTEState::Failure;
		AuthorityResult.Message = FText::FromString(TEXT("Failed to start authority-driven QTE."));
		FinishFromResult(AuthorityResult);
		return false;
	}
	bWaitingForInstigator = false;
	ShowQTEActivityDisplay(Instigator);
	return true;
}

void UQTEActivityStep::HandleAuthorityQTEFinished(int32 RequestId, const FQTEAuthorityResult& AuthorityResult)
{
	UE_LOG(MS_QTEActivityStep, Log, TEXT("HandleAuthorityQTEFinished request=%d activeRequest=%d outcome='%d' completed=%d mistakes=%d message='%s'."),
		RequestId,
		ActiveRequestId,
		static_cast<int32>(AuthorityResult.Outcome),
		AuthorityResult.CompletedStepCount,
		AuthorityResult.Mistakes,
		*AuthorityResult.Message.ToString());

	if (RequestId != ActiveRequestId)
	{
		return;
	}

	FinishFromResult(AuthorityResult);
}

void UQTEActivityStep::FinishFromResult(const FQTEAuthorityResult& AuthorityResult)
{
	HideQTEActivityDisplay();
	UnbindAuthorityDelegate();

	StepResult.Status = ResolveStepStatus(AuthorityResult.Outcome);
	StepResult.Score = AuthorityResult.Outcome == EQTEState::Success
		? FMath::Max(AuthorityResult.CompletedStepCount, 1)
		: AuthorityResult.CompletedStepCount;

	FinishStep(StepResult);
}

EActivityStepStatus UQTEActivityStep::ResolveStepStatus(EQTEState FinalState) const
{
	switch (FinalState)
	{
	case EQTEState::Success:
		return EActivityStepStatus::Success;
	case EQTEState::Failure:
	case EQTEState::Timeout:
	case EQTEState::Canceled:
	case EQTEState::Interrupted:
	default:
		return EActivityStepStatus::Fail;
	}
}

void UQTEActivityStep::ShowQTEActivityDisplay(AActor* Instigator)
{
	if (!Instigator || !Instigator->Implements<UQTEActivityDisplay>())
	{
		ActiveDisplayActor.Reset();
		return;
	}

	ActiveDisplayActor = Instigator;
	IQTEActivityDisplay::Execute_ShowQTEActivityStep(Instigator, ActiveQTEComponent.Get(), QTEWidgetClass);
}

void UQTEActivityStep::HideQTEActivityDisplay()
{
	AActor* DisplayActor = ActiveDisplayActor.Get();
	if (DisplayActor && DisplayActor->Implements<UQTEActivityDisplay>())
	{
		IQTEActivityDisplay::Execute_HideQTEActivityStep(DisplayActor);
	}

	ActiveDisplayActor.Reset();
}

void UQTEActivityStep::UnbindAuthorityDelegate()
{
	if (ActiveQTEComponent.IsValid())
	{
		ActiveQTEComponent->OnQTEAuthorityFinished.RemoveAll(this);
	}

	ActiveQTEComponent.Reset();
	ActiveRequestId = INDEX_NONE;
	bWaitingForInstigator = false;
}
