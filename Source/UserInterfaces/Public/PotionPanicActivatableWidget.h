#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "PotionPanicActivatableWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class USERINTERFACES_API UPotionPanicActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:

	UPotionPanicActivatableWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintImplementableEvent, Category = "Input", meta = (DisplayName = "On Back Action"))
	void BP_OnBackAction();

protected:

	virtual void NativeOnInitialized() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	virtual bool NativeOnBackAction() { return false; }

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FName DesiredFocusWidgetName;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	ECommonInputMode InputMode = ECommonInputMode::Menu;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	EMouseCaptureMode MouseCaptureMode = EMouseCaptureMode::NoCapture;
};
