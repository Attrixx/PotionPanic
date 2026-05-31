#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "PotionPanicOptionCycle.generated.h"

class UButton;

UCLASS(Abstract, Blueprintable)
class USERINTERFACES_API UPotionPanicOptionCycle : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

	UFUNCTION()
	UWidget* HandleCycleNavigation(EUINavigation Direction);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BTN_Previous;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> BTN_Next;

	UPROPERTY(EditAnywhere, Category = "Option", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "1.2"))
	float HighlightScale = 1.05f;

private:

	void SetHighlighted(bool bHighlighted);
};
