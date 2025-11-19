#include "RecipeSystem/StationComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/StrategyInterface.h"

TObjectPtr<AActor> UStationComponent::GetItemOnSocket()
{
	TSet<UActorComponent*> ActorComponents = GetOwner()->GetComponents();
	for (UActorComponent* Component : ActorComponents)
	{
		if (USocketComponent* Socket = Cast<USocketComponent>(Component))
		{
			if (Socket->IsHolding())
			{
				return Socket->GetHeld()->GetOwner();
			}
		}
	}
	return nullptr;
}

void UStationComponent::Interact(APawn* Instigator)
{
	StartProcessItem(Instigator);
}

void UStationComponent::StartProcessItem(APawn* Instigator)
{
	// Get item from socket
	TObjectPtr<AActor> InputItem = GetItemOnSocket();
	OnBeginProcess.Broadcast(Instigator, InputItem);

	int ItemIndex = 0;
	if (InputItem != nullptr && InputItems.Num())
	{
		ItemIndex = InputItems.Find(InputItem->GetClass());
	}

	// Get corresponding output item
	TSubclassOf<AActor> OutputItem = nullptr;
	if (OutputItems.Num() > ItemIndex)
	{
		OutputItem = OutputItems[ItemIndex];
	}

	// Process item with strategies
	if (Strategies.Num() > ItemIndex && Strategies[ItemIndex] != nullptr && Strategies[ItemIndex]->GetClass()->ImplementsInterface(UStrategyInterface::StaticClass()))
	{
		if (IStrategyInterface* Strategy = Cast<IStrategyInterface>(Strategies[ItemIndex]))
		{
			Strategy->ExecuteStrategy(this, InputItem->GetClass(), OutputItem);
		}
	}

	// Notify when done
	OnEndProcess.Broadcast(Instigator, OutputItem);
}
