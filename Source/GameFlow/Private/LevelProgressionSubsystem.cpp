#include "LevelProgressionSubsystem.h"
#include "LevelProgressionSaveGame.h"
#include "Kismet/GameplayStatics.h"

void ULevelProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadProgression();
}

void ULevelProgressionSubsystem::Deinitialize()
{
	SaveProgression();
	Super::Deinitialize();
}

void ULevelProgressionSubsystem::LoadProgression()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		CurrentSaveGame = Cast<ULevelProgressionSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	}
	
	if (!CurrentSaveGame)
	{
		CurrentSaveGame = Cast<ULevelProgressionSaveGame>(UGameplayStatics::CreateSaveGameObject(ULevelProgressionSaveGame::StaticClass()));
	}
}

void ULevelProgressionSubsystem::SaveProgression()
{
	if (CurrentSaveGame)
	{
		UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, SaveUserIndex);
	}
}

FLevelData ULevelProgressionSubsystem::GetLevelData(FName LevelID, UDataTable* StaticDataTable)
{
	FLevelData ResultData;
	
	// Default fallbacks in case data isn't found
	ResultData.LevelName = LevelID.ToString();
	ResultData.LevelNumber = 0.0f;
	ResultData.bIsLocked = true;
	ResultData.Score = 0.0f;
	ResultData.LevelTexture = nullptr;
	ResultData.TargetScoreToUnlockNextLevel = 100.0f;

	if (!LevelID.IsNone())
	{
		// 1. Fetch Static Data
		if (StaticDataTable)
		{
			FString ContextString = TEXT("LevelProgressionSubsystem::GetLevelData");
			if (FLevelStaticData* StaticData = StaticDataTable->FindRow<FLevelStaticData>(LevelID, ContextString))
			{
				ResultData.LevelName = StaticData->LevelName;
				ResultData.LevelNumber = StaticData->LevelNumber;
				// Load the texture synchronously if needed, or just pass the soft pointer (here we assume we want it loaded if asked)
				ResultData.LevelTexture = StaticData->LevelTexture.LoadSynchronous();
				ResultData.TargetScoreToUnlockNextLevel = StaticData->TargetScoreToUnlockNextLevel;
			}
		}

		// 2. Fetch Dynamic Data
		if (CurrentSaveGame && CurrentSaveGame->SavedLevelsData.Contains(LevelID))
		{
			const FLevelDynamicData& DynamicData = CurrentSaveGame->SavedLevelsData[LevelID];
			ResultData.bIsLocked = DynamicData.bIsLocked;
			ResultData.Score = DynamicData.Score;
		}
	}

	return ResultData;
}

bool ULevelProgressionSubsystem::IsLevelUnlocked(FName LevelID)
{
	if (!CurrentSaveGame || LevelID.IsNone()) return false;

	FLevelDynamicData& LevelData = CurrentSaveGame->SavedLevelsData.FindOrAdd(LevelID);
	return LevelData.bIsLocked;
}

TArray<FName> ULevelProgressionSubsystem::CheckLevelsToUnlock(UDataTable* StaticDataTable)
{
	TArray<FName> LevelsToUnlock;

	if (!IsValid(StaticDataTable)) return LevelsToUnlock;

	TArray<int> LevelsToUnlockByNumber;
	TMap<int, FName> LevelNumberToIDMap;
	for (const auto& Row : StaticDataTable->GetRowMap())
	{
		const FName& LevelID = Row.Key;
		const FLevelData& LevelData = GetLevelData(LevelID, StaticDataTable);
		LevelNumberToIDMap.Add(LevelData.LevelNumber, LevelID);
		if (LevelData.LevelNumber == 1 && LevelData.bIsLocked)
		{
			LevelsToUnlock.Add(LevelID);
		}
		if (LevelData.Score >= LevelData.TargetScoreToUnlockNextLevel)
		{
			LevelsToUnlockByNumber.Add(LevelData.LevelNumber + 1);
		}
	}

	for (int LevelNumber : LevelsToUnlockByNumber)
	{
		if (LevelNumberToIDMap.Contains(LevelNumber))
		{
			FName LevelIDToUnlock = LevelNumberToIDMap[LevelNumber];
			FLevelData LevelData = GetLevelData(LevelIDToUnlock, StaticDataTable);
			if (LevelData.bIsLocked)
			{
				LevelsToUnlock.Add(LevelIDToUnlock);
			}
		}
	}

	return LevelsToUnlock;
}

void ULevelProgressionSubsystem::UnlockLevel(FName LevelID)
{
	UnlockLevels(TArray<FName>{ LevelID });
}

void ULevelProgressionSubsystem::UnlockLevels(const TArray<FName>& LevelIDs)
{
	if (!CurrentSaveGame) return;

	bool bAnyLevelUnlocked = false;
	for (const FName& LevelID : LevelIDs)
	{
		if (LevelID.IsNone()) continue;
		FLevelDynamicData& LevelData = CurrentSaveGame->SavedLevelsData.FindOrAdd(LevelID);
		if (LevelData.bIsLocked)
		{
			LevelData.bIsLocked = false;
			bAnyLevelUnlocked = true;
		}
	}
	if (bAnyLevelUnlocked)
	{
		SaveProgression();
	}
}

void ULevelProgressionSubsystem::UpdateLevelScore(FName LevelID, float NewScore)
{
	if (!CurrentSaveGame || LevelID.IsNone()) return;

	FLevelDynamicData& LevelData = CurrentSaveGame->SavedLevelsData.FindOrAdd(LevelID);
	if (NewScore > LevelData.Score)
	{
		LevelData.Score = NewScore;
		SaveProgression();
	}
}
