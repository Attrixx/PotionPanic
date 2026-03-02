#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemSpawnLibrary.generated.h"

class AItemActor;
class UItemAsset;

/**
 * Stateless helper for item spawn/destruction.
 * Keeps item lifecycle operations reusable without station/game-flow coupling.
 */
UCLASS()
class ITEMS_API UItemSpawnLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Items|Lifecycle", meta = (WorldContext = "WorldContextObject"))
	static AItemActor* SpawnItemFromAsset(
		UObject* WorldContextObject,
		UItemAsset* ItemAsset,
		const FTransform& SpawnTransform,
		TSubclassOf<AItemActor> ItemClass);

	UFUNCTION(BlueprintCallable, Category = "Items|Lifecycle", meta = (WorldContext = "WorldContextObject"))
	static AItemActor* SpawnItemFromPrimaryAssetId(
		UObject* WorldContextObject,
		FPrimaryAssetId ItemAssetId,
		const FTransform& SpawnTransform,
		TSubclassOf<AItemActor> ItemClass);

	UFUNCTION(BlueprintCallable, Category = "Items|Lifecycle")
	static void DestroyItemActor(AItemActor* ItemActor, bool bPlayFeedback = true);

	// TODO (Nath): Add pooled spawn path when the project introduces item pooling.
};
