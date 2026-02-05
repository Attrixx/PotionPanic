#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemProvider.generated.h"

class UItemAsset;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UItemProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for any actor/object that holds a high-level "Item" concept.
 * Allows decoupling logical item checks from physical ItemActors.
 */
class COREGAMEPLAY_API IItemProvider
{
	GENERATED_BODY()

public:
	/** Returns the Item Data Asset associated with this object */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Item")
	UItemAsset* GetItemAsset() const;
};
