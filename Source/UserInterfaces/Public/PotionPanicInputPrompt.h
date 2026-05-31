#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "PotionPanicInputPrompt.generated.h"

class UInputAction;
class UCommonActionWidget;
class UCommonTextBlock;
enum class ECommonInputType : uint8;

UCLASS(Abstract, Blueprintable)
class USERINTERFACES_API UPotionPanicInputPrompt : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Input Prompt")
	void SetInputAction(UInputAction* InInputAction);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Input Prompt")
	UInputAction* GetInputAction() const { return InputAction; }

protected:

	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Input Prompt")
	TObjectPtr<UCommonActionWidget> GamepadIcon;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Input Prompt")
	TObjectPtr<UCommonTextBlock> KeyboardText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Prompt")
	TObjectPtr<UInputAction> InputAction;

private:

	void HandleInputMethodChanged(ECommonInputType NewInputType);
	void RefreshPrompt();
	FKey ResolveKeyboardKey() const;

	FDelegateHandle InputMethodChangedHandle;
};
