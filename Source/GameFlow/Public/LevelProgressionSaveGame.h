#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "LevelProgressionTypes.h"
#include "LevelProgressionSaveGame.generated.h"

/**
 * SaveGame object for Level Progression (scores, unlocked status).
 */
UCLASS()
class GAMEFLOW_API ULevelProgressionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:

	ULevelProgressionSaveGame();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Level Progression")
	TMap<FName, FLevelDynamicData> SavedLevelsData;
};
