#pragma once

#include "CoreMinimal.h"
#include "KeybindTypes.generated.h"

USTRUCT(BlueprintType)
struct FKeybindEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind")
	FName InputActionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind")
	int32 MappingIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind")
	FKey KeyboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind")
	FKey GamepadKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind")
	FKey DefaultKeyboardKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keybind")
	FKey DefaultGamepadKey;

	FName GetUniqueId() const
	{
		return FName(*FString::Printf(TEXT("%s_%d"), *InputActionName.ToString(), MappingIndex));
	}
};
