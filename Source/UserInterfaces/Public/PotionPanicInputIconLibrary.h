#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PotionPanicInputIconLibrary.generated.h"

class ULocalPlayer;
class UImage;
class UTextBlock;
struct FSlateBrush;

UCLASS()
class USERINTERFACES_API UPotionPanicInputIconLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "PotionPanic|Input")
	static bool GetGamepadIconForKey(const ULocalPlayer* LocalPlayer, FKey Key, FSlateBrush& OutBrush);

	UFUNCTION(BlueprintCallable, Category = "PotionPanic|Input")
	static void ApplyKeyDisplay(UImage* IconImage, UTextBlock* KeyText, const ULocalPlayer* LocalPlayer, FKey Key);
};
