#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "GenericStation.generated.h"

class UStationDataAsset;

/**
 * Generic station configured entirely by StationDataAsset.
 */
UCLASS()
class STATIONS_API AGenericStation : public AStationActorBase
{
	GENERATED_BODY()

public:
	AGenericStation();
	virtual void OnConstruction(const FTransform& Transform) override;

protected:
	virtual void BeginPlay() override;
#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	void ApplyStationData();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	TObjectPtr<UStationDataAsset> StationData;
};
