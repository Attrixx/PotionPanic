#pragma once

#include "CoreMinimal.h"
#include "StationActorBase.h"
#include "GenericStation.generated.h"

class UStationDataAsset;

/**
 * A generic station that configures itself based on a Data Asset.
 * Replaces specialized hardcoded station classes.
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

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UStationDataAsset> StationData;

private:
	void ApplyStationData();
};
