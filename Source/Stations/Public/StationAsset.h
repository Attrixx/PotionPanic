#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "StationAsset.generated.h"

UCLASS()
class STATIONS_API UStationAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	FText StationName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor")
	FName HolderSocket = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station", meta=(Categories="Activity"))
	FGameplayTagContainer ImplementedActivities;
};
