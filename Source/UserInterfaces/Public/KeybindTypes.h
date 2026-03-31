#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
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

	FString GetUniqueId() const
	{
		return FString::Printf(TEXT("%s_%d"), *InputActionName.ToString(), MappingIndex);
	}
};

UCLASS()
class USERINTERFACES_API UKeybindsSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TArray<FKeybindEntry> SavedBindings;

	static const FString SlotName;
};
