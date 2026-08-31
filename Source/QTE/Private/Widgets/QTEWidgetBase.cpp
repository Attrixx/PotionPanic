#include "Widgets/QTEWidgetBase.h"
#include "Components/QTEComponent.h"

void UQTEWidgetBase::BindToQTEComponent(UQTEComponent* InQTEComponent)
{
	if (BoundQTEComponent == InQTEComponent)
	{
		return;
	}

	UQTEComponent* PreviousQTEComponent = BoundQTEComponent.Get();
	UnbindFromCurrentComponent();
	BoundQTEComponent = InQTEComponent;
	OnBoundQTEComponentChanged(PreviousQTEComponent, BoundQTEComponent.Get());

	if (!BoundQTEComponent)
	{
		return;
	}

	BoundQTEComponent->OnQTEStarted.AddDynamic(this, &UQTEWidgetBase::HandleQTEStarted);
	BoundQTEComponent->OnQTEStepChanged.AddDynamic(this, &UQTEWidgetBase::HandleQTEStepChanged);
	BoundQTEComponent->OnQTEUpdated.AddDynamic(this, &UQTEWidgetBase::HandleQTEUpdated);
	BoundQTEComponent->OnQTEFinished.AddDynamic(this, &UQTEWidgetBase::HandleQTEFinished);

	const FQTERuntimeState State = BoundQTEComponent->GetCurrentQTEState();
	if (State.Status == EQTEState::Running)
	{
		HandleQTEStarted(State);
		HandleQTEStepChanged(State);
	}
	HandleQTEUpdated(State);
}

UQTEComponent* UQTEWidgetBase::GetBoundQTEComponent() const
{
	return BoundQTEComponent;
}

void UQTEWidgetBase::NativeDestruct()
{
	UQTEComponent* PreviousQTEComponent = BoundQTEComponent.Get();
	UnbindFromCurrentComponent();
	OnBoundQTEComponentChanged(PreviousQTEComponent, nullptr);
	Super::NativeDestruct();
}

void UQTEWidgetBase::OnBoundQTEComponentChanged(UQTEComponent* PreviousQTEComponent, UQTEComponent* NewQTEComponent)
{
}

void UQTEWidgetBase::HandleQTEStarted_Implementation(const FQTERuntimeState& State)
{
}

void UQTEWidgetBase::HandleQTEStepChanged_Implementation(const FQTERuntimeState& State)
{
}

void UQTEWidgetBase::HandleQTEUpdated_Implementation(const FQTERuntimeState& State)
{
}

void UQTEWidgetBase::HandleQTEFinished_Implementation(const FQTEResult& Result)
{
}

void UQTEWidgetBase::UnbindFromCurrentComponent()
{
	if (!BoundQTEComponent)
	{
		return;
	}

	BoundQTEComponent->OnQTEStarted.RemoveDynamic(this, &UQTEWidgetBase::HandleQTEStarted);
	BoundQTEComponent->OnQTEStepChanged.RemoveDynamic(this, &UQTEWidgetBase::HandleQTEStepChanged);
	BoundQTEComponent->OnQTEUpdated.RemoveDynamic(this, &UQTEWidgetBase::HandleQTEUpdated);
	BoundQTEComponent->OnQTEFinished.RemoveDynamic(this, &UQTEWidgetBase::HandleQTEFinished);
	BoundQTEComponent = nullptr;
}
