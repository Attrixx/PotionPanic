// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/GameplayAbilitySystem/PotionPanicTags.h"

namespace PotionPanicTags
{
	namespace Abilities
	{
		UE_DEFINE_GAMEPLAY_TAG(Dash, "Abilities.Dash");
		UE_DEFINE_GAMEPLAY_TAG(Drop, "Abilities.Drop");
		UE_DEFINE_GAMEPLAY_TAG(Interact, "Abilities.Interact");
		UE_DEFINE_GAMEPLAY_TAG(PickUp, "Abilities.PickUp");
		UE_DEFINE_GAMEPLAY_TAG(Throw, "Abilities.Throw");
		UE_DEFINE_GAMEPLAY_TAG(TransformItem, "Abilities.TransformItem");
	}

	namespace Cooldown
	{
		UE_DEFINE_GAMEPLAY_TAG(Dash, "Cooldown.Dash");
	}

	namespace Character
	{
		namespace State
		{
			UE_DEFINE_GAMEPLAY_TAG(Carrying, "Character.State.Carrying");
			UE_DEFINE_GAMEPLAY_TAG(Dashing, "Character.State.Dashing");
			UE_DEFINE_GAMEPLAY_TAG(Interacting, "Character.State.Interacting");
			UE_DEFINE_GAMEPLAY_TAG(Stunned, "Character.State.Stunned");
			UE_DEFINE_GAMEPLAY_TAG(CanPickUpItem, "Character.State.CanPickUpItem");
			UE_DEFINE_GAMEPLAY_TAG(CanInteract, "Character.State.CanInteract");
		}
	}

	namespace GameplayCues
	{
		UE_DEFINE_GAMEPLAY_TAG(Throw, "GameplayCue.Throw");
		UE_DEFINE_GAMEPLAY_TAG(Dash, "GameplayCue.Dash");
		UE_DEFINE_GAMEPLAY_TAG(PickUp, "GameplayCue.PickUp");
		UE_DEFINE_GAMEPLAY_TAG(Drop, "GameplayCue.Drop");
	}

	namespace Stations
	{
		UE_DEFINE_GAMEPLAY_TAG(Cauldron, "Stations.Cauldron");
		UE_DEFINE_GAMEPLAY_TAG(CuttingBoard, "Stations.CuttingBoard");
		UE_DEFINE_GAMEPLAY_TAG(Spawner, "Stations.Spawner");
		namespace Spawners
		{
			UE_DEFINE_GAMEPLAY_TAG(Sugar, "Stations.Spawner.Sugar");
			UE_DEFINE_GAMEPLAY_TAG(Verbena, "Stations.Spawner.Verbena");
			UE_DEFINE_GAMEPLAY_TAG(FairyDust, "Stations.Spawner.FairyDust");
			UE_DEFINE_GAMEPLAY_TAG(Flask, "Stations.Spawner.Flask");
		}
	}
}