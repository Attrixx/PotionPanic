#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemComponent.generated.h"

class UStationComponent;
class URecipe;

UCLASS(Abstract, Blueprintable)
class UItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	virtual void OnStartConsuming(UStationComponent& Consumer, URecipe& Recipe) {};
	virtual void OnConsumed() {};

	UPROPERTY(EditAnywhere)
	FText Name;
};
