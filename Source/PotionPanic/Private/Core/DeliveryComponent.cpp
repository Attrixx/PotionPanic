
#include "Core/DeliveryComponent.h"
#include "OrderSystem/OrderClient.h"
#include "Core/SocketComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

UDeliveryComponent::UDeliveryComponent()
{

}

void UDeliveryComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedOrderClient = ResolveOrderClient();
}

TObjectPtr<USocketComponent> UDeliveryComponent::GetSocket(AActor* TargetActor)
{
	TSet<UActorComponent*> ActorComponents = TargetActor->GetComponents();
	for (UActorComponent* Component : ActorComponents)
	{
		if (USocketComponent* Socket = Cast<USocketComponent>(Component))
		{
			if (!Socket->IsHolding()) continue;

			return Socket;
		}
	}
	return nullptr;
}

TObjectPtr<AOrderClient> UDeliveryComponent::ResolveOrderClient() const
{
	if (LinkedOrderClient)
	{
		return LinkedOrderClient;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AOrderClient> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void UDeliveryComponent::Deliver(APawn* Instigator, TObjectPtr<AActor> Item)
{
	if (!Item)
	{
		return;
	}

	// Get Socket
	TObjectPtr<USocketComponent> TargetSocket = GetSocket(GetOwner());
	if (!TargetSocket) return;

	// Discard held item
	TargetSocket->Take();

	if (!CachedOrderClient || CachedOrderClient->IsActorBeingDestroyed())
	{
		CachedOrderClient = ResolveOrderClient();
	}

	if (CachedOrderClient)
	{
		CachedOrderClient->TryServeDish(Item);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DeliveryComponent::Deliver - Aucun client de commande valide trouve."));
	}

	Item->Destroy();

	UE_LOG(LogTemp, Log, TEXT("Delivered an item"));
}
