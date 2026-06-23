#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "KeybindTypes.h"
#include "ControlsSettingsWidget.generated.h"

class UInputMappingContext;
class UKeybindManager;
class UWidget;
class FRebindKeyPreprocessor;
enum class ECommonInputType : uint8;

UCLASS()
class USERINTERFACES_API UControlsSettingsWidget : public UPotionPanicActivatableWidget
{
	GENERATED_BODY()

	UControlsSettingsWidget(const FObjectInitializer& ObjectInitializer);

public:

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void InitializeBindings();

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void StartRebinding(FName ActionName, int32 MappingIndex);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void CancelRebinding();

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void ResetBinding(FName ActionName, int32 MappingIndex);

	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void ResetAllBindings();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Controls")
	TArray<FKeybindEntry> GetBindings(bool bGamepad) const;

	UFUNCTION(BlueprintPure, Category = "Settings|Controls")
	bool IsUsingGamepad() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Controls")
	void OnBindingsRefreshed(const TArray<FKeybindEntry>& Bindings);

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Controls")
	void OnDuplicateKeyDetected(FKey DuplicateKey, const FText& ConflictingActionName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Controls")
	void OnKeyRebound(FName ActionName, int32 MappingIndex, FKey NewKey);

	void ApplyIfDirty();

protected:

	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Controls")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Controls")
	TMap<FName, FText> DisplayNameOverrides;

private:

	void ProcessRebind(const FKey& NewKey);
	void Apply();

	void RegisterRebindPreprocessor();
	void UnregisterRebindPreprocessor();
	void HandleRebindKeyCaptured(FKey CapturedKey);

	TSharedPtr<FRebindKeyPreprocessor> RebindPreprocessor;

	void RefreshList();
	void HandleInputMethodChanged(ECommonInputType NewInputType);

	void FocusListDeferred();
	void FocusListNow();
	UWidget* FindRowFocusWidget(FName ActionName, int32 MappingIndex) const;
	UWidget* FindFirstRowFocusWidget() const;

	FName PendingFocusActionName   = NAME_None;
	int32 PendingFocusMappingIndex = 0;

	FDelegateHandle InputMethodChangedHandle;

	UPROPERTY()
	TObjectPtr<UKeybindManager> KeybindManager;

	bool  bIsListeningForInput  = false;
	FName RebindingActionName   = NAME_None;
	int32 RebindingMappingIndex = 0;
};
