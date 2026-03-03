#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CoreGameplay/Public/ActivityAsset.h"
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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Station")
	TArray<TObjectPtr<UActivityAsset>> Activities;
};
