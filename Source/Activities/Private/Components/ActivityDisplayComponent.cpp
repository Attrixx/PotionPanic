// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/ActivityDisplayComponent.h"
#include "ActivityDisplaySettings.h"
#include "ActivityExecutor.h"
#include "ActivityStep.h"
#include "Widgets/ActivityStepWidget.h"
#include <Camera/PlayerCameraManager.h>
#include <GameFramework/PlayerController.h>
#include <Materials/MaterialInterface.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ActivityDisplayComponent, Verbose, All);

UActivityDisplayComponent::UActivityDisplayComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	SetWidgetSpace(EWidgetSpace::World);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
	SetTwoSided(true);

	SetVisibility(false);
	SetTickMode(ETickMode::Disabled);
}

void UActivityDisplayComponent::BeginPlay()
{
	const UActivityDisplaySettings* Settings = GetDefault<UActivityDisplaySettings>();

	SetDrawAtDesiredSize(Settings->bSizeFromWidget);
	if (!Settings->bSizeFromWidget)
	{
		SetDrawSize(FVector2D(Settings->WidgetDrawSize));
	}

	SetRelativeLocation(Settings->WidgetRelativeLocation);
	SetRelativeScale3D(FVector(Settings->WidgetScale));

	if (UMaterialInterface* Material = Settings->WidgetMaterial.LoadSynchronous())
	{
		SetMaterial(0, Material);
	}

	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	Executor = GetOwner() ? GetOwner()->FindComponentByClass<UActivityExecutor>() : nullptr;
	if (!Executor)
	{
		UE_LOGFMT(MS_ActivityDisplayComponent, Warning, "'{0}' has no activity executor to display.",
			GetNameSafe(GetOwner()));
		return;
	}

	Executor->OnStepPresentationChanged.AddDynamic(this, &UActivityDisplayComponent::Executor_OnStepPresentationChanged);
	ApplyPresentation(Executor->GetStepPresentation());
}

void UActivityDisplayComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Executor)
	{
		Executor->OnStepPresentationChanged.RemoveAll(this);
		Executor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UActivityDisplayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AlignToLocalCamera();
}

void UActivityDisplayComponent::AlignToLocalCamera()
{
	const UWorld* World = GetWorld();
	const APlayerController* LocalController = World ? World->GetFirstPlayerController() : nullptr;
	const APlayerCameraManager* CameraManager = LocalController ? LocalController->PlayerCameraManager : nullptr;
	if (!CameraManager)
		return;

	const FQuat CameraRotation = CameraManager->GetCameraRotation().Quaternion();
	const FVector FacingViewer = -CameraRotation.GetForwardVector();

	SetWorldRotation(FRotationMatrix::MakeFromXZ(FacingViewer, CameraRotation.GetUpVector()).Rotator());
}

void UActivityDisplayComponent::Executor_OnStepPresentationChanged(UActivityExecutor* InExecutor)
{
	check(InExecutor == Executor);
	ApplyPresentation(InExecutor->GetStepPresentation());
}

void UActivityDisplayComponent::ApplyPresentation(const FActivityStepPresentation& Presentation)
{
	if (Presentation.StepClass != CurrentStepClass)
	{
		CurrentStepClass = Presentation.StepClass;
		const UActivityDisplaySettings* Settings = GetDefault<UActivityDisplaySettings>();
		UClass* StepWidgetClass = Settings->GetWidgetClass(CurrentStepClass).LoadSynchronous();

		if (CurrentStepClass && !StepWidgetClass)
		{
			UE_LOGFMT(MS_ActivityDisplayComponent, Warning, "No widget mapped to step '{0}' in the Activity Display settings; it will run without one.",
				GetNameSafe(CurrentStepClass));
		}

		SetVisibility(StepWidgetClass != nullptr);
		SetWidgetClass(StepWidgetClass);
		SetTickMode(StepWidgetClass ? ETickMode::Enabled : ETickMode::Disabled);

		if (StepWidgetClass)
		{
			AlignToLocalCamera();
		}
	}

	if (auto* StepWidget = Cast<UActivityStepWidget>(GetWidget()))
	{
		StepWidget->SetPresentation(Presentation);
	}
}
