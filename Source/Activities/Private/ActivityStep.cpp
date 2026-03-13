#include "ActivityStep.h"

void UActivityStep::FinishStep(const FActivityStepResult& Output) const
{
	ActivityFinishedCallback.ExecuteIfBound(Output);
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
