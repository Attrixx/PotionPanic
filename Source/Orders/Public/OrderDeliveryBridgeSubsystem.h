#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OrderDeliveryBridgeSubsystem.generated.h"

class AActor;
class ADeliveryStation;

UCLASS()
class ORDERS_API UOrderDeliveryBridgeSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Orders|Delivery")
	bool IsAuthorityWorld() const;

	UFUNCTION(BlueprintCallable, Category = "Orders|Delivery")
	void RefreshDeliveryStations();

	UFUNCTION(BlueprintCallable, Category = "Orders|Delivery")
	void RegisterDeliveryStation(ADeliveryStation* DeliveryStation);

	UFUNCTION(BlueprintCallable, Category = "Orders|Delivery")
	void UnregisterDeliveryStation(ADeliveryStation* DeliveryStation);

	UFUNCTION(BlueprintPure, Category = "Orders|Delivery")
	int32 GetBoundDeliveryStationCount() const;

private:
	UFUNCTION()
	void HandleDeliveryItem(FPrimaryAssetId DeliveredItemId, AActor* SourceStation);

	void HandleActorSpawned(AActor* SpawnedActor);

private:
	TSet<TWeakObjectPtr<ADeliveryStation>> BoundStations;
	FDelegateHandle ActorSpawnedHandle;
};
