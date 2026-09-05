#include "ActivityStep.h"
#include "ActivityExecutor.h"
#include <GameFramework/GameStateBase.h>

void UActivityStep::FinishStep(const FActivityStepResult& Output) const
{
	StepFinishedCallback.ExecuteIfBound(Output);
}

UActivityExecutor* UActivityStep::GetExecutor() const
{
	return GetTypedOuter<UActivityExecutor>();
}

double UActivityStep::GetServerTimeSeconds() const
{
	const UWorld* World = GetOuter() ? GetOuter()->GetWorld() : nullptr;
	const AGameStateBase* GameState = World ? World->GetGameState() : nullptr;

	// Clients read the same clock through their own game state, which is what makes a replicated
	// deadline mean the same thing on both sides.
	return GameState ? GameState->GetServerWorldTimeSeconds() : 0.0;
}

void UActivityStep::StartStep_Implementation(AActor* LastInstigator)
{
}

void UActivityStep::OnInteract_Implementation(AActor* Instigator)
{
}

void UActivityStep::CancelStep_Implementation()
{
}

void UActivityStep::OnActivityInput_Implementation(AActor* Instigator, EActivityInputSlot Slot)
{
}

void UActivityStep::OnCancelRequested_Implementation(AActor* Instigator)
{
}
