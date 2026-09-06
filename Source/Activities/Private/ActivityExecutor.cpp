// Fill out your copyright notice in the Description page of Project Settings.

#include "ActivityExecutor.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ActivityAsset.h"
#include "ActivityConclusion.h"
#include "ActivityEvaluator.h"
#include "ActivityStep.h"
#include "ActivityStepSettings.h"
#include <Net/UnrealNetwork.h>

#include "GameFramework/GameStateBase.h"

DEFINE_LOG_CATEGORY_STATIC(MS_ActivityExecutor, Log, All);

UActivityExecutor::UActivityExecutor()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UActivityExecutor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UActivityExecutor, State);
	DOREPLIFETIME(UActivityExecutor, Presentation);
}

void UActivityExecutor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (State.Holder.IsValid())
	{
		Cancel();
		State.Holder->OnCarriableChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

bool UActivityExecutor::IsAuthority() const
{
	return GetOwner() && GetOwner()->HasAuthority();
}

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

void UActivityExecutor::StartActivity(UActivityAsset* Activity, AActor* Instigator, bool bItemTakenFromInstigator)
{
	if (!IsAuthority())
		return;

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
	State.bItemTakenFromInstigator = bItemTakenFromInstigator;

	// A fresh activity starts with a single instigator, whoever it turns out to be.
	State.LastInstigator = nullptr; // null instigator is valid
	State.bInstigatorChanged = false;
	RefreshInstigator(Instigator);

	Steps.Reset(Activity->ActivitySteps.Num()); // Step.Num() == 0 is valid
	for (UActivityStepSettings* Settings : Activity->ActivitySteps)
	{
		check(Settings);

		if (UActivityStep* Step = Settings->CreateStep(this))
		{
			Step->StepFinishedCallback.BindUObject(this, &UActivityExecutor::OnStepFinished);
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
		Conclude(EActivityExecutionStatus::Success);
		return;
	}

	Evaluator = Activity->Evaluator;
	if (!IsValid(Evaluator))
	{
		UE_LOGFMT(MS_ActivityExecutor, Error, "Invalid Evaluator.");
		return;
	}

	Reset(EActivityExecutionStatus::Ongoing);
	ContinueExecution();
}

void UActivityExecutor::Interact(AActor* Instigator)
{
	if (IsAuthority() && State.Status == EActivityExecutionStatus::Ongoing)
	{
		RefreshInstigator(Instigator);

		check(CurrentStepIndex < Steps.Num());
		Steps[CurrentStepIndex]->OnInteract(Instigator);
	}
}

void UActivityExecutor::Cancel()
{
	if (IsAuthority() && State.Status == EActivityExecutionStatus::Ongoing)
	{
		check(CurrentStepIndex < Steps.Num());
		if (bCurrentStepStarted)
		{
			Steps[CurrentStepIndex]->CancelStep();
		}
		Reset(EActivityExecutionStatus::Canceled);
	}
}

EActivityExecutionStatus UActivityExecutor::GetExecutionStatus() const
{
	return State.Status;
}

void UActivityExecutor::ReceiveActivityInput(AActor* Instigator, EActivityInputSlot Slot)
{
	if (!IsAuthority())
		return;

	if (State.Status != EActivityExecutionStatus::Ongoing)
	{
		// The step that captured this actor is already over: its last presses are still in flight.
		UE_LOGFMT(MS_ActivityExecutor, Verbose, "Activity input from '{0}' ignored, nothing is running.",
			GetNameSafe(Instigator));
		return;
	}

	check(CurrentStepIndex < Steps.Num());
	Steps[CurrentStepIndex]->OnActivityInput(Instigator, Slot);
}

double UActivityExecutor::GetServerTimeSeconds() const
{
	const UWorld* World = GetOuter() ? GetOuter()->GetWorld() : nullptr;
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;
	return GameState ? GameState->GetServerWorldTimeSeconds() : 0.0;
}

void UActivityExecutor::ClearStepPresentation()
{
	if (IsAuthority() && Presentation.StepClass)
	{
		Presentation.StepClass = nullptr;
		UpdatePresentation();
	}
}

void UActivityExecutor::RefreshInstigator(AActor* Instigator)
{
	if (Instigator && Instigator != State.LastInstigator.Get())
	{
		State.bInstigatorChanged |= !State.LastInstigator.IsExplicitlyNull();
		State.LastInstigator = Instigator;
	}

	AActor* Current = State.LastInstigator.Get();
	UHolderComponent* Holder = Current ? Current->FindComponentByClass<UHolderComponent>() : nullptr;

	State.InstigatorHolder = Holder;
	State.InstigatorItem = Holder ? Cast<AItemActor>(Holder->GetCarriable()) : nullptr;
}

void UActivityExecutor::Holder_OnCarriableChanged(UHolderComponent* Holder)
{
	check(Holder && Holder == State.Holder);

	if (!IsAuthority())
		return;

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
		bCurrentStepStarted = true;
		Steps[CurrentStepIndex]->StartStep(State.LastInstigator.Get());
		
		Presentation.StepClass = Steps[CurrentStepIndex].GetClass();
		Presentation.Instigator = State.LastInstigator.Get();
		Presentation.StartServerTime = GetServerTimeSeconds();
		UpdatePresentation();
	}
}

void UActivityExecutor::OnStepFinished(const FActivityStepResult& StepResult)
{
	if (State.Status != EActivityExecutionStatus::Ongoing)
	{
		// We were probably canceled, but this should not happen (the step did not clean up itself)
		UE_LOGFMT(MS_ActivityExecutor, Warning, "OnStepFinished called but the activity is not ongoing.");
		return;
	}

	FActivityEvaluationResult EvalResult = Evaluator->EvaluateStep(State, StepResult);
	State.Score = EvalResult.Score;

	switch (EvalResult.FlowDecision)
	{
	case EActivityFlowDecision::Continue:
		bCurrentStepStarted = false;
		if (++CurrentStepIndex >= Steps.Num())
		{
			Conclude(EActivityExecutionStatus::Success);
			return; // do not call ContinueExecution
		}
		break;

	case EActivityFlowDecision::Fail:
		Conclude(EActivityExecutionStatus::Failed);
		return; // do not call ContinueExecution

	case EActivityFlowDecision::Restart:
		// WARN: If we later want the restart to create again the steps (if there is random in Settings::CreateStep)
		// then we Cancel/StartActivity again
		Reset(EActivityExecutionStatus::Ongoing);
		break;

	default:
		checkNoEntry();
	}

	ContinueExecution();
}

void UActivityExecutor::Conclude(EActivityExecutionStatus Status)
{
	// Status: 0 NotStarted, 1 Ongoing, 2 Success, 3 Failed, 4 Canceled.
	UE_LOGFMT(MS_ActivityExecutor, Log, "Conclude on {0}: status {1}, score {2}, item {3}, conclusion {4}.",
		GetNameSafe(GetOwner()),
		static_cast<int32>(Status),
		State.Score,
		GetNameSafe(State.Item.Get()),
		GetNameSafe(Conclusion));

	State.Status = Status;
	ClearStepPresentation();
	Conclusion->Conclude(State);
	OnExecutionStatusChanged.Broadcast(this, Status);
}

void UActivityExecutor::Reset(EActivityExecutionStatus Status)
{
	// Steps take their own presentation down when they end, but not when they are cancelled from
	// under them, and a widget left on screen outlives everything it was describing.
	ClearStepPresentation();

	State.Status = Status;
	State.Score = 0;
	CurrentStepIndex = 0;
	bCurrentStepStarted = false;
	OnExecutionStatusChanged.Broadcast(this, Status);
}

void UActivityExecutor::OnRep_State(const FActivityExecutionState& OldState)
{
	if (OldState.Status != State.Status)
	{
		OnExecutionStatusChanged.Broadcast(this, State.Status);
	}
}

void UActivityExecutor::UpdatePresentation()
{
	++Presentation.Revision;
	OnStepPresentationChanged.Broadcast(this);
}

void UActivityExecutor::OnRep_Presentation()
{
	OnStepPresentationChanged.Broadcast(this);
}
