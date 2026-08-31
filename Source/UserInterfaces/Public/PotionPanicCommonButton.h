#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "PotionPanicCommonButton.generated.h"

UCLASS(Abstract, Blueprintable)
class USERINTERFACES_API UPotionPanicCommonButton : public UCommonButtonBase
{
	GENERATED_BODY()

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;
	virtual void HandleFocusReceived() override;
	virtual void HandleFocusLost() override;
	virtual void NativeOnSelected(bool bBroadcast) override;
	virtual void NativeOnDeselected(bool bBroadcast) override;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHighlight(bool bHighlighted);

private:

	void RefreshHighlight();

	bool bIsHoveredHighlight = false;
	bool bIsFocusedHighlight = false;
	bool bIsHighlighted = false;
};
