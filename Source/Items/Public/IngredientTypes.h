#pragma once

#include "CoreMinimal.h"
#include "IngredientTypes.generated.h"

UENUM(BlueprintType)
enum class EIngredientType : uint8
{
    Raw UMETA(DisplayName = "Raw"),
    Processed UMETA(DisplayName = "Processed")
};

/**
 * Defines how an ingredient is retrieved from its source.
 * Metadata used by Spawners/Stations to determine interaction logic.
 */
UENUM(BlueprintType)
enum class ERetrievalType : uint8
{
    Pickup UMETA(DisplayName = "Pickup"),
    Interaction UMETA(DisplayName = "Interaction"),
    QTE UMETA(DisplayName = "QTE"),
    Destructible UMETA(DisplayName = "Destructible")
};

UENUM(BlueprintType)
enum class ERetrievalDifficulty : uint8
{
    Simple UMETA(DisplayName = "Simple"),
    Medium UMETA(DisplayName = "Medium"),
    Hard UMETA(DisplayName = "Hard")
};

USTRUCT(BlueprintType)
struct ITEMS_API FQTEData
{
    GENERATED_BODY()

    // Number of successes required to complete the QTE
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 SuccessCount = 3;

    // Time allowed to complete the QTE (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxDuration = 5.0f;

    // Identifier for the input action required (optional, depends on system)
    // UPROPERTY(EditAnywhere, BlueprintReadOnly)
    // UInputAction* RequiredInput;
};

USTRUCT(BlueprintType)
struct ITEMS_API FDestructionData
{
    GENERATED_BODY()

    // If true, this item breaks when hitting a surface with sufficient force
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsFragile = false;

    // Impulse threshold to trigger destruction/damage (for Fragile items)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bIsFragile"))
    float ImpactThreshold = 500.0f;

    // Total health if multiple hits are required (e.g. Ice block)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MaxHealth = 100.0f;
};
