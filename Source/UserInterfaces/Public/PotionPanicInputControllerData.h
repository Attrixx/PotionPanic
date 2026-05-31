#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "PotionPanicInputControllerData.generated.h"

UCLASS(BlueprintType, Abstract)
class USERINTERFACES_API UPotionPanicInputControllerData : public UCommonInputBaseControllerData
{
	GENERATED_BODY()
};

UCLASS(BlueprintType)
class USERINTERFACES_API UPotionPanicXboxInputData : public UPotionPanicInputControllerData
{
	GENERATED_BODY()
public:
	UPotionPanicXboxInputData()
	{
		InputType = ECommonInputType::Gamepad;
		GamepadName = TEXT("XBoxOne");
		GamepadDisplayName = NSLOCTEXT("PotionPanic", "GamepadXbox", "Xbox Controller");
	}
};

UCLASS(BlueprintType)
class USERINTERFACES_API UPotionPanicPSInputData : public UPotionPanicInputControllerData
{
	GENERATED_BODY()
public:
	UPotionPanicPSInputData()
	{
		InputType = ECommonInputType::Gamepad;
		GamepadName = TEXT("PS5");
		GamepadDisplayName = NSLOCTEXT("PotionPanic", "GamepadPS", "PlayStation Controller");
	}
};
