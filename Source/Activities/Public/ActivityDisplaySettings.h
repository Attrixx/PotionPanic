// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ActivityStepPresentation.h"
#include "ActivityDisplaySettings.generated.h"

class UActivityStep;
class UActivityStepWidget;
class UMaterialInterface;

/** Project-wide widgets used to draw running activity steps, keyed by the step class they draw. */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Activity Display"))
class ACTIVITIES_API UActivityDisplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	FName GetCategoryName() const override;

	/** @return Widget class configured for StepClass or its nearest mapped parent; null if none is mapped. */
	TSoftClassPtr<UActivityStepWidget> GetWidgetClass(TSubclassOf<UActivityStep> StepClass) const;

	/** Which widget draws which step; a step with no row inherits its parent's. */
	UPROPERTY(EditAnywhere, config, Category = "Widgets")
	TMap<TSubclassOf<UActivityStep>, TSoftClassPtr<UActivityStepWidget>> StepWidgets;

	/** If true, the quad follows the widget's Slate desired size and WidgetDrawSize is ignored. */
	UPROPERTY(EditAnywhere, config, Category = "Placement")
	bool bSizeFromWidget = true;

	/** Render resolution in pixels when not sizing from widget; also the quad's size in world units before scaling. */
	UPROPERTY(EditAnywhere, config, Category = "Placement", meta = (EditCondition = "!bSizeFromWidget"))
	FVector2D WidgetDrawSize = FVector2D(400.f, 200.f);

	/** Uniform scale applied to the quad without affecting its render resolution. */
	UPROPERTY(EditAnywhere, config, Category = "Placement", meta = (ClampMin = 0.001))
	float WidgetScale = 1.f;

	/** Where the widget sits relative to the actor running the activity. */
	UPROPERTY(EditAnywhere, config, Category = "Placement")
	FVector WidgetRelativeLocation = FVector(0.f, 0.f, 120.f);

	/** Material the widget quad is drawn with; unset falls back to engine default. */
	UPROPERTY(EditAnywhere, config, Category = "Rendering")
	TSoftObjectPtr<UMaterialInterface> WidgetMaterial;
};
