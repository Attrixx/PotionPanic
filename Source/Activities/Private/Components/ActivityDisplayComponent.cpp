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

	// After the camera has moved for this frame: aligning to the previous frame's camera is a
	// widget that visibly lags behind whenever the view turns.
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	SetWidgetSpace(EWidgetSpace::World);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);

	// The quad is single-sided by default, so a billboard turned the wrong way round is not a
	// mirrored widget but no widget at all -- it gets backface culled and fails silently. Two-sided
	// costs nothing here and turns that failure into something you can actually see.
	SetTwoSided(true);

	// Nothing is running when the level opens, and an empty quad floating over every station is
	// worse than no widget at all. Both go back on together in ApplyPresentation.
	//
	// SetTickMode, never SetComponentTickEnabled: UWidgetComponent draws to its render target from
	// its own tick and drives that tick from TickMode, so a tick enabled behind its back is one it
	// switches off again the next time it looks.
	SetVisibility(false);
	SetTickMode(ETickMode::Disabled);
}

void UActivityDisplayComponent::BeginPlay()
{
	const UActivityDisplaySettings* Settings = GetDefault<UActivityDisplaySettings>();

	SetDrawAtDesiredSize(Settings->bSizeFromWidget);
	if (!Settings->bSizeFromWidget)
	{
		// Before Super, not after: UWidgetComponent::BeginPlay copies DrawSize into the
		// CurrentDrawSize it actually renders and sizes its Slate window with, so setting it
		// afterwards is a no-op until something else happens to refresh it. Sizing from the widget
		// has no such problem: it recomputes that on every draw.
		SetDrawSize(FVector2D(Settings->WidgetDrawSize));
	}

	SetRelativeLocation(Settings->WidgetRelativeLocation);
	SetRelativeScale3D(FVector(Settings->WidgetScale));

	// Drives UpdateMaterialInstance, which rebuilds the dynamic instance and re-pushes the render
	// target into it, so the override needs no setup here beyond being assigned.
	if (UMaterialInterface* Material = Settings->WidgetMaterial.LoadSynchronous())
	{
		SetMaterial(0, Material);
	}

	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		// Nobody is watching this one. The executor still publishes, it just goes over the wire.
		return;
	}

	Executor = GetOwner() ? GetOwner()->FindComponentByClass<UActivityExecutor>() : nullptr;
	if (!Executor)
	{
		UE_LOGFMT(MS_ActivityDisplayComponent, Warning, "'{0}' has no activity executor to display.",
			GetNameSafe(GetOwner()));
		return;
	}

	UE_LOGFMT(MS_ActivityDisplayComponent, Log, "Listening to the activity executor on '{0}'.", GetNameSafe(GetOwner()));
	Executor->OnStepPresentationChanged.AddDynamic(this, &UActivityDisplayComponent::Executor_OnStepPresentationChanged);

	// A late joiner receives the presentation before this runs, and would otherwise show nothing
	// until the next change.
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

	// Parallel to the view plane rather than turned towards the camera's position: every station
	// draws its widget at the same angle, and one sitting off to the side of the screen is not
	// skewed by the perspective.
	//
	// The readable face is the quad's +X, so it points back down the view direction rather than
	// along it. Built from the camera's axes instead of a 180 degree yaw: yaw turns about the world
	// up, which under a pitched camera tilts the widget away instead of flipping it over.
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

		// Synchronous: the step is already running, and a widget streamed in a frame late is a
		// window the player watched a piece of without seeing it.
		UClass* StepWidgetClass = Settings->GetWidgetClass(CurrentStepClass).LoadSynchronous();

		// Log, not Verbose: this fires twice per step, and it is the line that says whether a step
		// reached this machine at all and whether the settings had a widget for it.
		UE_LOGFMT(MS_ActivityDisplayComponent, Log, "Presentation on '{0}': step '{1}' -> widget '{2}'.",
			GetNameSafe(GetOwner()),
			GetNameSafe(CurrentStepClass),
			GetNameSafe(StepWidgetClass));

		if (CurrentStepClass && !StepWidgetClass)
		{
			UE_LOGFMT(MS_ActivityDisplayComponent, Warning, "No widget mapped to step '{0}' in the Activity Display settings; it will run without one.",
				GetNameSafe(CurrentStepClass));
		}

		// Visible before the widget is built, not after: both UpdateWidget() and ShouldDrawWidget()
		// read the component's visibility, and a widget built while hidden never makes it to the
		// render target.
		SetVisibility(StepWidgetClass != nullptr);
		SetWidgetClass(StepWidgetClass);

		// Enabled rather than Automatic: automatic lets the component switch its own tick off the
		// first time it judges the widget invisible, and nothing on this side turns it back on.
		SetTickMode(StepWidgetClass ? ETickMode::Enabled : ETickMode::Disabled);

		// Oriented right away, so it does not spend its first frame facing the wrong way.
		if (StepWidgetClass)
		{
			AlignToLocalCamera();
		}

		// Everything the widget needs in order to reach the screen, in one line: if it is on and
		// nothing shows, the problem is past this component.
		UE_LOGFMT(MS_ActivityDisplayComponent, Log, "  widget='{0}' isStepWidget={1} visible={2} ticking={3} drawSize={4}x{5}.",
			GetNameSafe(GetWidget()),
			Cast<UActivityStepWidget>(GetWidget()) != nullptr,
			IsVisible(),
			IsComponentTickEnabled(),
			GetCurrentDrawSize().X,
			GetCurrentDrawSize().Y);
	}

	if (auto* StepWidget = Cast<UActivityStepWidget>(GetWidget()))
	{
		StepWidget->SetPresentation(Presentation);
	}
}
