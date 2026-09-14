#include "ActivityStep.h"
#include "ActivityExecutor.h"

void UActivityStep::FinishStep(const FActivityStepResult& Output) const
{
	StepFinishedCallback.ExecuteIfBound(Output);
}

UActivityExecutor* UActivityStep::GetExecutor() const
{
	return GetTypedOuter<UActivityExecutor>();
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
