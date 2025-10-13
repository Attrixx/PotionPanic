#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StationComponent.generated.h"

UCLASS(Abstract, Blueprintable)
class UStationComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	FText Name;
};
