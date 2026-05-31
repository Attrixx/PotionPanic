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

	UPROPERTY(EditAnywhere, Category = "PotionPanic|Highlight", meta = (ClampMin = "1.0", UIMin = "1.0", UIMax = "1.3"))
	float HighlightScale = 1.1f;

private:

	void UpdateHighlight();

	bool bIsFocusedHighlight = false;
};
