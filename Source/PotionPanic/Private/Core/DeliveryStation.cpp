#include "Core/DeliveryStation.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "OrderSystem/OrderClient.h"
#include "OrderSystem/CommandeManagerWorldSubsystem.h"

ADeliveryStation::ADeliveryStation()
{
}

void ADeliveryStation::BeginPlay()
{
	Super::BeginPlay();

	if (SocketComponent)
	{
		SocketComponent->OnHeldChanged.AddUObject(this, &ADeliveryStation::Deliver);
	}
}

AOrderClient* ADeliveryStation::FindTargetClient() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	AOrderClient* Fallback = nullptr;

	for (TActorIterator<AOrderClient> It(World); It; ++It)
	{
		AOrderClient* Client = *It;
		if (!Fallback)
		{
			Fallback = Client;
		}
		if (Client && Client->HasActiveOrder())
		{
			return Client;
		}
	}

	return Fallback;
}

void ADeliveryStation::Deliver(USocketableComponent* OldHeld, USocketableComponent* NewHeld)
{
	if (OldHeld || !NewHeld)
		return;

	AActor* DishActor = NewHeld->GetOwner();
	if (!DishActor)
		return;

	AOrderClient* Client = FindTargetClient();
	if (!Client)
		return;

	bool bValid = false;

	if (UCommandeManagerWorldSubsystem* Sub = GetWorld()->GetSubsystem<UCommandeManagerWorldSubsystem>())
	{
		bValid = Sub->ValidateDishForClient(Client, DishActor);
	}
	else
	{
		bValid = Client->CheckDishMatchesCurrentOrder(DishActor);
	}

	if (!bValid)
		return;

	SocketComponent->Take();
	Client->TryServeDish(DishActor);
}
