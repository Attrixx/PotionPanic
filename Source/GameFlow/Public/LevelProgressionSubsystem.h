#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LevelProgressionTypes.h"
#include "LevelProgressionSubsystem.generated.h"

class ULevelProgressionSaveGame;
class UDataTable;

/**
 * Subsystem to manage level progression (loading/saving locked status and scores).
 */
UCLASS()
class GAMEFLOW_API ULevelProgressionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Get merged static and dynamic data for a level */
	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	FLevelData GetLevelData(FName LevelID, UDataTable* StaticDataTable);

	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	bool IsLevelUnlocked(FName LevelID);

	TArray<FName> CheckLevelsToUnlock(UDataTable* StaticDataTable);

	/** Unlock a specific level */
	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	void UnlockLevel(FName LevelID);

	/** Unlock multiple levels */
	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	void UnlockLevels(const TArray<FName>& LevelIDs);

	/** Update the score of a specific level (only saves if the score is strictly higher) */
	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	void UpdateLevelScore(FName LevelID, float NewScore);

	/** Save the current progression to disk */
	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	void SaveProgression();

private:

	void LoadProgression();

	UPROPERTY()
	TObjectPtr<ULevelProgressionSaveGame> CurrentSaveGame;

	FString SaveSlotName = TEXT("LevelProgressionSlot");
	int32 SaveUserIndex = 0;
};
