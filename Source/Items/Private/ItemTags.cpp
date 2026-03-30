#include "ItemTags.h"

namespace GameTags
{
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item,
	"Item",
	"Parent tag for all Items. Items are required for certain Transformations (Recipes)."
	);
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_None,
	"Item.None",
	"Indicates a requirement of no item. Useful in recipes that spawn items."
);
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Ingredient,
	"Item.Ingredient",
	"Ingredient Items are used as input in Transformations."
);
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Product,
	"Item.Product",
	"Product Items are in their final form, ready for delivery."
);
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Utensil,
	"Item.Utensil",
	"Utensil Items should never be destroyed."
);
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Utensil_Container,
	"Item.Utensil.Container",
	"Container Items are Utensils Items that can contain Ingredient Items."
);
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Item_Breakable,
	"Item.Breakable",
	"Breakables Items must be handled with care."
);
}
