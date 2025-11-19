
#include "Core/DeliveryComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "GameFramework/Actor.h"

UDeliveryComponent::UDeliveryComponent()
{

}

void UDeliveryComponent::BeginPlay()
{
	Super::BeginPlay();
}

TObjectPtr<USocketComponent> UDeliveryComponent::GetSpawnSocket(AActor* TargetActor)
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

void UDeliveryComponent::Deliver(APawn* Instigator, TObjectPtr<AActor> Item)
{
	// Get Socket
	TObjectPtr<USocketComponent> TargetSocket = GetSpawnSocket(GetOwner());
	if (!TargetSocket) return;

	//GetWorld()->GetSubsystem<UScoreSubsystem>()->AddScore(24);
	Item->Destroy();
}

