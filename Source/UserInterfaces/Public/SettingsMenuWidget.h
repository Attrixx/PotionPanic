#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UWidgetSwitcher;
class UVideoSettingsWidget;
class UAudioSettingsWidget;
class UControlsSettingsWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackClicked);

UCLASS()
class USERINTERFACES_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Settings")
	FOnBackClicked OnBackClicked;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAndClose();

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WS_ContentSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVideoSettingsWidget> WBP_VideoSettings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UAudioSettingsWidget> WBP_AudioSettings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UControlsSettingsWidget> WBP_ControlsSettings;
};