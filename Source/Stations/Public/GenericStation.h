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
	void ApplyStationData();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UStationDataAsset> StationData;

	// TODO (Nath): Add editor validation for missing StationData fields (mesh/instructions/activities).
};
