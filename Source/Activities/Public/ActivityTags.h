#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace GameTags
{

// Activity is condition
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Activity);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Activity_Spawner); // Activity that has no Input item
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Activity_Destroy); // Activity that has no Output item
}
