#include "Core/CamTargetComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

UCamTargetComponent::UCamTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCamTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegister)
	{
		Register();
	}
}

void UCamTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Unregister();

	Super::EndPlay(EndPlayReason);
}

void UCamTargetComponent::Register()
{
	Registry.AddUnique(this);
}

void UCamTargetComponent::Unregister()
{
	Registry.RemoveSwap(this);
}