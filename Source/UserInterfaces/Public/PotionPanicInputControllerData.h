#pragma once

#include "CoreMinimal.h"
#include "CommonInputBaseTypes.h"
#include "PotionPanicInputControllerData.generated.h"

UCLASS(BlueprintType)
class USERINTERFACES_API UPotionPanicXboxInputData : public UCommonInputBaseControllerData
{
	GENERATED_BODY()
public:
	UPotionPanicXboxInputData()
	{
		InputType = ECommonInputType::Gamepad;
		GamepadName = TEXT("XBox");
		GamepadDisplayName = NSLOCTEXT("PotionPanic", "GamepadXbox", "Xbox Controller");
	}
};

UCLASS(BlueprintType)
class USERINTERFACES_API UPotionPanicPSInputData : public UCommonInputBaseControllerData
{
	GENERATED_BODY()
public:
	UPotionPanicPSInputData()
	{
		InputType = ECommonInputType::Gamepad;
		GamepadName = TEXT("PlayStation");
		GamepadDisplayName = NSLOCTEXT("PotionPanic", "GamepadPS", "PlayStation Controller");
	}
};
