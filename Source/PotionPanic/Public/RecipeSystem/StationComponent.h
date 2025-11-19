#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/InteractionInterface.h"
#include "StationComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnBeginProcessDelegate, APawn*, TObjectPtr<AActor>);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnEndProcessDelegate, APawn*, TSubclassOf<AActor>);

class UStrategyInterface;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UStationComponent : public UActorComponent, public IInteractionInterface
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere)
	FText Name;

	FOnBeginProcessDelegate OnBeginProcess;
	FOnEndProcessDelegate OnEndProcess;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<AActor>> InputItems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UObject>> Strategies;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<AActor>> OutputItems;

protected:

	TObjectPtr<AActor> GetItemOnSocket();

public:

	void Interact(APawn* Instigator) override;

	UFUNCTION(BlueprintCallable)
	void StartProcessItem(APawn* Instigator);

};
