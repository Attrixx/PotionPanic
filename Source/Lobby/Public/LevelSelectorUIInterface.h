#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LevelSelectorUIInterface.generated.h"

UINTERFACE(MinimalAPI)
class ULevelSelectorUIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for UI elements inside a Level Selector, allowing the actor to communicate with them
 * without creating circular dependencies.
 */
class LOBBY_API ILevelSelectorUIInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, Category = "Level Progression")
	void SetLevelNumber(int32 LevelNumber);

	/** Called when the level is unlocked during gameplay */
	UFUNCTION(BlueprintNativeEvent, Category = "Level Progression")
	void OnLevelUnlocked();
};
