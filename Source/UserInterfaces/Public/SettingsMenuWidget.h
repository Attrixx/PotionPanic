#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "Input/UIActionBindingHandle.h"
#include "SettingsMenuWidget.generated.h"

class UWidgetSwitcher;
class UVideoSettingsWidget;
class UAudioSettingsWidget;
class UControlsSettingsWidget;
class UCommonButtonBase;
class UInputAction;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackClicked);

UCLASS()
class USERINTERFACES_API USettingsMenuWidget : public UPotionPanicActivatableWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Settings")
	FOnBackClicked OnBackClicked;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void ApplyAndClose();

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SelectTab(int32 Index);

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnBackAction() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> TabNextAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> TabPrevAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MenuInputContext;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WS_ContentSwitcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UVideoSettingsWidget> WBP_VideoSettings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UAudioSettingsWidget> WBP_AudioSettings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UControlsSettingsWidget> WBP_ControlsSettings;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> WBP_Video;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> WBP_Audio;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonButtonBase> WBP_Controls;

private:

	void HandleTabNext();
	void HandleTabPrev();

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonButtonBase>> Tabs;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCommonActivatableWidget>> Contents;

	TArray<FUIActionBindingHandle> TabActionBindings;

	int32 CurrentTabIndex = 0;
};
