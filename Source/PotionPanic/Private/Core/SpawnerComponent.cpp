
#include "Core/SpawnerComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/PotionPanicCharacter.h"
#include "GameFramework/Actor.h"

USpawnerComponent::USpawnerComponent()
	: bSpawnOnInstigatorSocket(false)
{

}

void USpawnerComponent::BeginPlay()
{
	Super::BeginPlay();
}

TObjectPtr<USocketComponent> USpawnerComponent::GetSpawnSocket(AActor* TargetActor)
{
	TSet<UActorComponent*> ActorComponents = TargetActor->GetComponents();
	for (UActorComponent* Component : ActorComponents)
	{
		if (USocketComponent* Socket = Cast<USocketComponent>(Component))
		{
			if (Socket->IsHolding()) continue;

			return Socket;
		}
	}
	return nullptr;
}

void USpawnerComponent::SpawnItem(APawn* Instigator, TSubclassOf<AActor> Item)
{
	// Get Socket
	AActor* TargetActor = bSpawnOnInstigatorSocket && Instigator ? Instigator : GetOwner();
	TObjectPtr<USocketComponent> TargetSocket = GetSpawnSocket(TargetActor);
	if (!TargetSocket) return;

	// Spawn Item
	TObjectPtr<AActor> SpawnedItem = GetWorld()->SpawnActor<AActor>(Item, TargetSocket->GetComponentLocation(), TargetSocket->GetComponentRotation());

	// Destroy previous item
	if (!bSpawnOnInstigatorSocket)
	{
		USocketableComponent* ItemComponent = TargetSocket->Take();
		if (ItemComponent)
		{
			AActor* ItemActor = ItemComponent->GetOwner();
			ItemActor->Destroy();
		}
	}

	// Put on socket
	USocketableComponent* Socketable = SpawnedItem->GetComponentByClass<USocketableComponent>();
	if (Socketable)
	{
		TargetSocket->Put(*Socketable);
	}
}

