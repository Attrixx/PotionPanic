#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "LevelProgressionTypes.generated.h"

class UWorld;

USTRUCT(BlueprintType)
struct FLevelStaticData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText LevelDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int LevelNumber;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> LevelPreviewTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> LevelLoadingTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetScoreToUnlockNextLevel = 0.0f;
};

USTRUCT(BlueprintType)
struct FLevelDynamicData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Score = 0.0f;
};

USTRUCT(BlueprintType)
struct FLevelData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText LevelDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int LevelNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Score = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UTexture2D> LevelTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetScoreToUnlockNextLevel = 0.0f;
};
