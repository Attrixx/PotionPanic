// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityExecutor.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ActivityAsset.h"
#include "ActivityConclusion.h"
#include "ActivityEvaluator.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ActivityExecutor, Log, All);

void UActivityExecutor::Initialize(UHolderComponent* Holder)
{
	if (!IsValid(Holder))
	{
		UE_LOGFMT(MS_ActivityExecutor, Warning, "Initialize with null Holder ignored.");
		return;
	}

	if (State.Holder.IsValid())
	{
		Cancel();
		State.Holder->OnCarriableChanged.RemoveAll(this);
	}

	State.Holder = Holder;
	Holder->OnCarriableChanged.AddDynamic(this, &UActivityExecutor::Holder_OnCarriableChanged);
}

void UActivityExecutor::StartActivity(UActivityAsset* Activity, AActor* Instigator)
{
	if (!State.Holder.IsValid())
	{
		UE_LOGFMT(MS_ActivityExecutor, Error, "Cannot StartActivity. Holder is not initialized.");
		return;
	}

	if (!IsValid(Activity))
	{
		UE_LOGFMT(MS_ActivityExecutor, Warning, "StartActivity with null Activity ignored.");
		return;
	}

	Cancel();

	State.Item = Cast<AItemActor>(State.Holder->GetCarriable()); // null item is valid
	State.LastInstigator = Instigator; // null instigator is valid

	Steps.Empty(Activity->ActivitySteps.Num()); // Step.Num() == 0 is valid
	for (UActivityStepSettings* Settings : Activity->ActivitySteps)
	{
		check(Settings);

		if (UActivityStep* Step = Settings->CreateStep(this))
		{
			Step->ActivityFinishedCallback.BindUObject(this, &UActivityExecutor::OnStepFinished);
			Steps.Emplace(Step);
		}
		else
		{
			UE_LOGFMT(MS_ActivityExecutor,
				Warning,
				"CreateStep failed on ActivityStepSettings {1} in Activity {0}.",
				Activity->GetName(),
				Settings->GetName()
			);
		}
	}

	Conclusion = Activity->Conclusion;
	if (!IsValid(Conclusion))
	{
		UE_LOGFMT(MS_ActivityExecutor, Error, "Invalid conclusion.");
		return;
	}

	if (Steps.Num() == 0)
	{
		State.Status = EActivityExecutionStatus::Success;
		Conclusion->Conclude(State);
		return;
	}

	Evaluator = Activity->Evaluator;
	if (!IsValid(Evaluator))
	{
		UE_LOGFMT(MS_ActivityExecutor, Error, "Invalid Evaluator.");
		return;
	}

	State.Status = EActivityExecutionStatus::Ongoing;
	ContinueExecution();
}

void UActivityExecutor::Interact(AActor* Instigator)
{
	if (State.Status == EActivityExecutionStatus::Ongoing)
	{
		check(CurrentStepIndex < Steps.Num());
		State.LastInstigator = Instigator;
		Steps[CurrentStepIndex]->OnInteract(Instigator);
	}
}

void UActivityExecutor::Cancel()
{
	if (State.Status == EActivityExecutionStatus::Ongoing)
	{
		check(CurrentStepIndex < Steps.Num());
		Steps[CurrentStepIndex]->CancelStep();
	}
	Reset();
}

EActivityExecutionStatus UActivityExecutor::GetExecutionStatus() const
{
	return State.Status;
}

void UActivityExecutor::Holder_OnCarriableChanged(UHolderComponent* Holder)
{
	check(Holder && Holder == State.Holder);

	if (Holder->GetCarriable() != State.Item)
	{
		Cancel();
		State.Item = Cast<AItemActor>(Holder->GetCarriable());
	}
}

void UActivityExecutor::ContinueExecution()
{
	if (State.Status == EActivityExecutionStatus::Ongoing)
	{
		check(CurrentStepIndex < Steps.Num());
		Steps[CurrentStepIndex]->StartStep(State.LastInstigator.Get());
	}
}

void UActivityExecutor::OnStepFinished(const FActivityStepResult& StepResult)
{
	if (State.Status != EActivityExecutionStatus::Ongoing)
	{
		// We were probably canceled, but this should not happen
		UE_LOGFMT(MS_ActivityExecutor, Warning, "OnStepFinished called but the activity is not ongoing.");
		return;
	}

	FActivityEvaluationResult EvalResult = Evaluator->EvaluateStep(State, StepResult);
	State.Score = EvalResult.Score;

	switch (EvalResult.FlowDecision)
	{
	case EActivityFlowDecision::Continue:
		if (++CurrentStepIndex >= Steps.Num())
		{
			State.Status = EActivityExecutionStatus::Success;
			Conclusion->Conclude(State);
			return; // do not call ContinueExecution
		}
		break;

	case EActivityFlowDecision::Fail:
		State.Status = EActivityExecutionStatus::Failed;
		Conclusion->Conclude(State);
		return; // do not call ContinueExecution

	case EActivityFlowDecision::Restart:
		Reset();
		State.Status = EActivityExecutionStatus::Ongoing;
		break;
		
	default:
		checkNoEntry();
	}
	
	ContinueExecution();
}

void UActivityExecutor::Reset()
{
	State.Status = EActivityExecutionStatus::NotStarted;
	State.Score = 0;
	CurrentStepIndex = 0;
}
