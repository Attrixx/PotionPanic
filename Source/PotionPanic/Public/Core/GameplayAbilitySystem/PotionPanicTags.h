// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace PotionPanicTags
{
	namespace Abilities
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Drop);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interact);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(PickUp);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Throw);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(TransformItem);
	}

	namespace Cooldown
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);
	}

	namespace Character
	{
		namespace State
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Carrying);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dashing);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Interacting);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Stunned);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(CanPickUpItem);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(CanInteract);
		}
	}

	namespace GameplayCues
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Throw);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Dash);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(PickUp);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Drop);
	}

	namespace Stations
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cauldron);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(CuttingBoard);
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(Spawner);
		namespace Spawners
		{
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Sugar);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Verbena);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(FairyDust);
			UE_DECLARE_GAMEPLAY_TAG_EXTERN(Flask);
		}
	}
}