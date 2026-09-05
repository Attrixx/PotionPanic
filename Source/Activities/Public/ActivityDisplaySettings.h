// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ActivityStepPresentation.h"
#include "ActivityDisplaySettings.generated.h"

class UActivityStep;
class UActivityStepWidget;
class UMaterialInterface;

/**
 * Project-wide widgets used to draw running activity steps, keyed by the step class they draw.
 *
 * They are global on purpose: a step describes what is happening, not what it looks like, and two
 * cauldrons running the same combo have no business drawing it differently. Per-station flavour
 * belongs inside the widget, which knows the station and the instigator it is drawing for.
 *
 * A new step type only has to add a row here -- nothing in this module enumerates the step kinds.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Activity Display"))
class ACTIVITIES_API UActivityDisplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	FName GetCategoryName() const override;

	/**
	 * @param StepClass The step being drawn.
	 * @return The widget class configured for StepClass or, failing that, for its nearest mapped
	 *         parent. Null when nothing along that chain is mapped.
	 */
	TSoftClassPtr<UActivityStepWidget> GetWidgetClass(TSubclassOf<UActivityStep> StepClass) const;

	/**
	 * Which widget draws which step. A step with no row of its own inherits its parent's, so
	 * subclassing a step -- in C++ or in Blueprint -- does not require touching this.
	 *
	 * The step classes are hard references: they are loaded as soon as an activity mentions them
	 * anyway. The widgets stay soft, and are only pulled in when a step actually runs.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Widgets")
	TMap<TSubclassOf<UActivityStep>, TSoftClassPtr<UActivityStepWidget>> StepWidgets;

	/**
	 * Let each widget size itself: the quad follows its Slate desired size on every draw, and
	 * WidgetDrawSize is ignored.
	 * @note A widget whose root does not size itself -- a bare Canvas Panel, typically -- measures
	 *       zero and then renders nothing at all. Root those in something that has a desired size.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Placement")
	bool bSizeFromWidget = true;

	/**
	 * Resolution the widget is rendered at when it is not sizing itself, in pixels. Those pixels are
	 * also the quad's size in world units before scaling -- 400x200 is a four metre wide billboard
	 * at a scale of one -- so this is the sharpness knob and WidgetScale is the size one.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Placement", meta = (EditCondition = "!bSizeFromWidget"))
	FVector2D WidgetDrawSize = FVector2D(400.f, 200.f);

	/**
	 * Shrinks the quad without touching its resolution: at 0.1 a 400x200 widget is 40x20 cm in the
	 * world, still rendered with 400x200 pixels of detail. This is the only size knob left once the
	 * widget sizes itself.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Placement", meta = (ClampMin = 0.001))
	float WidgetScale = 1.f;

	/** Where the widget sits relative to the actor running the activity. */
	UPROPERTY(EditAnywhere, config, Category = "Placement")
	FVector WidgetRelativeLocation = FVector(0.f, 0.f, 120.f);

	/**
	 * Material the widget quad is drawn with. Left unset, the engine's depth-tested default is used
	 * and the widget is hidden by anything standing in front of it.
	 *
	 * Point this at a copy of Widget3DPassThrough_Translucent with "Disable Depth Test" ticked to
	 * draw it over the world instead. Whatever material is used has to carry the parameters the
	 * component drives: the SlateUI texture, plus TintColorAndOpacity and OpacityFromTexture.
	 */
	UPROPERTY(EditAnywhere, config, Category = "Rendering")
	TSoftObjectPtr<UMaterialInterface> WidgetMaterial;
};
