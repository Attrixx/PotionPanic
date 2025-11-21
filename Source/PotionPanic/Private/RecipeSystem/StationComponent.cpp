#include "RecipeSystem/StationComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/StrategyInterface.h"
#include "Core/StationActor.h"
#include "Core/PotionPanicCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"

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
	FGameplayEventData EventData;
	EventData.EventTag = FGameplayTag::RequestGameplayTag(FName("Abilities.Interact"));
	EventData.Instigator = Instigator;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), EventData.EventTag, EventData);

	APotionPanicCharacter* CharacterInstigator = Cast<APotionPanicCharacter>(Instigator);
	if (!IsValid(CharacterInstigator) || CharacterInstigator->IsHolding()) return;

	AStationActor* Station = Cast<AStationActor>(GetOwner());
	if (IsValid(Station))
	{
		Station->StartProcessing(Instigator, InternalStorage);
	}
	else
	{
		StartProcessItem(Instigator);
	}
}

void UStationComponent::StartProcessItem(APawn* Instigator)
{
	// Get item from socket
	TObjectPtr<AActor> InputItem = GetItemOnSocket();

	int RecipeIndex = -1;
	if (InputItems.Num() == 0) // Spawner case
	{
		RecipeIndex = 0;
	}
	else // Default case
	{
		if (InputItem) // Not cauldron
		{
			FInputItemGroup UniqueGroup{};
			UniqueGroup.Add(InputItem->GetClass());
			RecipeIndex = InputItems.Find(UniqueGroup);
		}
		else
		{
			RecipeIndex = InputItems.Find(InternalStorage);
		}
	}

	if (RecipeIndex == -1)
		return;

	// Get corresponding output item
	TSubclassOf<AActor> OutputItem = nullptr;
	if (OutputItems.Num() > RecipeIndex)
	{
		OutputItem = OutputItems[RecipeIndex];
	}

	// Process item with strategies
	if (Strategies.Num() > RecipeIndex && Strategies[RecipeIndex] != nullptr && Strategies[RecipeIndex]->GetClass()->ImplementsInterface(UStrategyInterface::StaticClass()))
	{
		if (IStrategyInterface* Strategy = Cast<IStrategyInterface>(Strategies[RecipeIndex]))
		{
			Strategy->ExecuteStrategy(this, InputItem->GetClass(), OutputItem);
		}
	}

	// Notify when done
	InternalStorage.Clear();
	OnEndProcess.Broadcast(Instigator, OutputItem);
}

void UStationComponent::Store(TSubclassOf<AActor> Item)
{
	InternalStorage.Add(Item);
	UE_LOGFMT(LogTemp, Log, "Current {0} storage: {1}", GetOwner()->GetName(), InternalStorage.Print());
}
