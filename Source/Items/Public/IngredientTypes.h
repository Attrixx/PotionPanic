#pragma once

#include "CoreMinimal.h"
#include "IngredientTypes.generated.h"

UENUM(BlueprintType)
enum class EIngredientType : uint8
{
	Raw UMETA(DisplayName = "Raw"),
	Processed UMETA(DisplayName = "Processed")
};

UENUM(BlueprintType)
enum class EIngredientStateFlag : uint8
{
	Cut UMETA(DisplayName = "Cut"),
	Crushed UMETA(DisplayName = "Crushed"),
	Infused UMETA(DisplayName = "Infused"),
	Burnt UMETA(DisplayName = "Burnt"),
	Frozen UMETA(DisplayName = "Frozen")
};

/**
 * Data-only descriptor of ingredient state.
 * No station/recipe gameplay logic should be encoded here.
 */
USTRUCT(BlueprintType)
struct ITEMS_API FIngredientStateDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|State")
	TArray<EIngredientStateFlag> StateFlags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ingredient|State")
	TArray<FName> StateTags;

	// TODO (Nath): Consolidate StateFlags + StateTags once a single project-wide tagging standard is chosen.
};
