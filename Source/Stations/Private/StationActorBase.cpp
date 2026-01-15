#include "StationActorBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStationActorBase::Interact(APlayerController& InInstigator)
{
	CurrentPlayer = &InInstigator;
}

void AStationActorBase::Execute(const FInstruction& Instruction)
{
	if (!CanExecuteActivity(Instruction.Activity))
	{
		UE_LOG(LogTemp, Warning, TEXT("Station cannot execute this activity"));
		return;
	}

	CurrentInstruction = Instruction;
	OnTransformationStarted();
}

bool AStationActorBase::CanExecuteActivity(UActivityAsset* Activity) const
{
	if (!Activity)
	{
		return false;
	}

	return Activities.Contains(Activity);
}

void AStationActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AStationActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTransforming)
	{
		TransformationProgress += DeltaTime;
		OnTransformationTick(DeltaTime);

		if (TransformationProgress >= TransformationDuration)
		{
			bIsTransforming = false;
			TransformationProgress = TransformationDuration;
			OnTransformationCompleted();
		}
	}
}

void AStationActorBase::StartTransformation(float Duration)
{
	if (bIsTransforming)
	{
		return;
	}

	bIsTransforming = true;
	TransformationProgress = 0.0f;
	TransformationDuration = Duration;
	OnTransformationStarted();
}

void AStationActorBase::CancelTransformation()
{
	if (!bIsTransforming)
	{
		return;
	}

	bIsTransforming = false;
	TransformationProgress = 0.0f;
	OnTransformationCancelled();
}

void AStationActorBase::OnTransformationStarted()
{
}

void AStationActorBase::OnTransformationTick(float DeltaTime)
{
}

void AStationActorBase::OnTransformationCompleted()
{
}

void AStationActorBase::OnTransformationCancelled()
{
}
