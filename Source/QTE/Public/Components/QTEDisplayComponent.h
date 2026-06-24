#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "QTEDisplayComponent.generated.h"

class UQTEComponent;
class UQTEWidgetBase;

/**
 * Floating widget shown above its owner while a QTEComponent (on the same actor, or passed in explicitly) is running a QTE.
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

private:

	UFUNCTION(Client, Reliable)
	void Client_ShowQTEActivityStep(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass);

	UFUNCTION(Client, Reliable)
	void Client_HideQTEActivityStep();
};
