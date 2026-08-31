#pragma once

#include "CoreMinimal.h"
#include "PotionPanicFocusHighlightWidget.h"
#include "Types/SlateEnums.h"
#include "PotionPanicOptionCycle.generated.h"

class UButton;

UCLASS(Abstract, Blueprintable)
class USERINTERFACES_API UPotionPanicOptionCycle : public UPotionPanicFocusHighlightWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	UWidget* HandleCycleNavigation(EUINavigation Direction);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BTN_Previous;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BTN_Next;
};
