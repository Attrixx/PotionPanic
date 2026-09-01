#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "QTEDisplayComponent.generated.h"

class UQTEComponent;
class UQTEWidgetBase;

/**
 * Floating widget shown while a QTEComponent (on the same actor, or passed in explicitly) is
 * running a QTE. For the duration of the QTE the widget anchors above the station the QTE
 * belongs to, then returns above its own owner.
 */
UCLASS(ClassGroup = (Gameplay), meta = (BlueprintSpawnableComponent))
class QTE_API UQTEDisplayComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UQTEDisplayComponent();

	void ShowQTEActivityStep(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass);
	void HideQTEActivityStep();

	UQTEWidgetBase* GetQTEWidget() const;

protected:

	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	/** The station the running QTE belongs to, or the owner when there is none. */
	AActor* ResolveAnchorActor(UQTEComponent* InQTEComponent) const;
	void AnchorAboveActor(AActor* AnchorActor);
	void RestoreOwnerAnchor();

	UFUNCTION(Client, Reliable)
	void Client_ShowQTEActivityStep(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass);

	UFUNCTION(Client, Reliable)
	void Client_HideQTEActivityStep();

private:

	/** Clearance above the anchor's bounds, so the medallion floats clear of the station mesh. */
	UPROPERTY(EditDefaultsOnly, Category = "QTE|Display", meta = (ClampMin = 0.0))
	float AnchorHeightClearance = 60.f;

	/** Relative location used when the widget sits above its own owner. */
	UPROPERTY(EditDefaultsOnly, Category = "QTE|Display")
	FVector OwnerRelativeLocation = FVector(0.f, 0.f, 180.f);
};
