#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace GameTags
{
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Ingredient); // Ingredients are used as input in transformations
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Product); // Item ready to be delivered
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Utensil); // Special item that should never be destroyed
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Utensil_Container);
}
