#include "ActivityTags.h"

namespace GameTags
{
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity,
	"Activity",
	"Parent tag for all Activities. Activities are required for Transformations (Recipes).");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Spawn,
	"Activity.Spawn",
	"Spawn Activities are associated with Transformations that do not consume an input.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Clear,
	"Activity.Clear",
	"Clear Activities are associated with Transformations that do not produce an output.");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(Activity_Morph,
	"Activity.Morph",
	"Morph Activities are associated with regular Transformations that turn an item into another.");
}
