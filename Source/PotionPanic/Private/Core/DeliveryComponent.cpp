
#include "Core/DeliveryComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "GameFramework/Actor.h"
#include "ScoreSystem/ScoreWorldSubsystem.h"

UDeliveryComponent::UDeliveryComponent()
{

}

void UDeliveryComponent::BeginPlay()
{
	Super::BeginPlay();
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

void UDeliveryComponent::Deliver(APawn* Instigator, TObjectPtr<AActor> Item)
{
	// Get Socket
	TObjectPtr<USocketComponent> TargetSocket = GetSocket(GetOwner());
	if (!TargetSocket) return;

	// Discard held item
	TargetSocket->Take();
	Item->Destroy();

	// Check if acceptable recipe
	// TODO FRANCOIS

	// Incr Score
	GetWorld()->GetSubsystem<UScoreWorldSubsystem>()->AddScore(1);

	UE_LOG(LogTemp, Log, TEXT("Delivered an item"));
}

