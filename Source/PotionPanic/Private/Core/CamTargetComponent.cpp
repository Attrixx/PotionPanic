#include "Core/CamTargetComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"

TArray<TWeakObjectPtr<UCamTargetComponent>> UCamTargetComponent::Registry;

UCamTargetComponent::UCamTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCamTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegister)
	{
		Registry.AddUnique(this);
	}
}

void UCamTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Registry.Remove(this);

	Super::EndPlay(EndPlayReason);
}

FVector UCamTargetComponent::ComputeLocation() const
{
	const AActor* Owner = GetOwner();
	if (!Owner) return FVector::ZeroVector;

	const USceneComponent* Root = Owner->GetRootComponent();
	if (Root && SocketName != NAME_None && Root->DoesSocketExist(SocketName))
	{
		return Root->GetSocketLocation(SocketName) + Offset;
	}
	return Owner->GetActorLocation() + Offset;
}

void UCamTargetComponent::GetAllTargets(const UObject* WorldContext, TArray<FVector>& OutLocations, bool bCleanup)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	OutLocations.Reset();
	if (!World) return;

	for (int32 i = Registry.Num() - 1; i >= 0; --i)
	{
		UCamTargetComponent* C = Registry[i].Get();
		if (!C || !C->GetOwner() || C->GetWorld() != World)
		{
			if (bCleanup) Registry.RemoveAtSwap(i);
			continue;
		}
		OutLocations.Add(C->ComputeLocation());
	}
}

int32 UCamTargetComponent::NumTargets(const UObject* WorldContext)
{
	TArray<FVector> Dummy;
	GetAllTargets(WorldContext, Dummy, false);
	return Dummy.Num();
}
