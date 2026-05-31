#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "KeybindTypes.h"
#include "ControlsSettingsWidget.generated.h"

class UInputMappingContext;
class UKeybindManager;

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Controls")
	void OnBindingsRefreshed(const TArray<FKeybindEntry>& Bindings);

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Controls")
	void OnDuplicateKeyDetected(FKey DuplicateKey, const FText& ConflictingActionName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Controls")
	void OnKeyRebound(FName ActionName, int32 MappingIndex, FKey NewKey);

	void ApplyIfDirty();

protected:

	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Controls")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Controls")
	TMap<FName, FText> DisplayNameOverrides;

private:

	void ProcessRebind(const FKey& NewKey);
	void Apply();

	UPROPERTY()
	TObjectPtr<UKeybindManager> KeybindManager;

	bool  bIsListeningForInput  = false;
	FName RebindingActionName   = NAME_None;
	int32 RebindingMappingIndex = 0;
};
