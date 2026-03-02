#include "OrderDeliveryBridgeSubsystem.h"
#include "DeliveryStation.h"
#include "Engine/World.h"
#include "OrderSystem.h"
#include "EngineUtils.h"

void UOrderDeliveryBridgeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	RefreshDeliveryStations();
	ActorSpawnedHandle = GetWorld()->AddOnActorSpawnedHandler(
		FOnActorSpawned::FDelegate::CreateUObject(this, &UOrderDeliveryBridgeSubsystem::HandleActorSpawned));
}

void UOrderDeliveryBridgeSubsystem::Deinitialize()
{
	if (GetWorld() != nullptr && ActorSpawnedHandle.IsValid())
	{
		GetWorld()->RemoveOnActorSpawnedHandler(ActorSpawnedHandle);
		ActorSpawnedHandle.Reset();
	}

	for (const TWeakObjectPtr<ADeliveryStation>& WeakStation : BoundStations)
	{
		if (ADeliveryStation* Station = WeakStation.Get())
		{
			Station->OnItemDelivered.RemoveDynamic(this, &UOrderDeliveryBridgeSubsystem::HandleDeliveryItem);
		}
	}
	BoundStations.Reset();

	Super::Deinitialize();
}

bool UOrderDeliveryBridgeSubsystem::IsAuthorityWorld() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_Standalone || NetMode == NM_ListenServer || NetMode == NM_DedicatedServer;
}

void UOrderDeliveryBridgeSubsystem::RefreshDeliveryStations()
{
	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	for (auto It = BoundStations.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
		}
	}

	for (TActorIterator<ADeliveryStation> It(GetWorld()); It; ++It)
	{
		RegisterDeliveryStation(*It);
	}
}

void UOrderDeliveryBridgeSubsystem::RegisterDeliveryStation(ADeliveryStation* DeliveryStation)
{
	if (!IsAuthorityWorld() || DeliveryStation == nullptr)
	{
		return;
	}

	if (BoundStations.Contains(DeliveryStation))
	{
		return;
	}

	DeliveryStation->OnItemDelivered.AddUniqueDynamic(this, &UOrderDeliveryBridgeSubsystem::HandleDeliveryItem);
	BoundStations.Add(DeliveryStation);
}

void UOrderDeliveryBridgeSubsystem::UnregisterDeliveryStation(ADeliveryStation* DeliveryStation)
{
	if (DeliveryStation == nullptr)
	{
		return;
	}

	DeliveryStation->OnItemDelivered.RemoveDynamic(this, &UOrderDeliveryBridgeSubsystem::HandleDeliveryItem);
	BoundStations.Remove(DeliveryStation);
}

int32 UOrderDeliveryBridgeSubsystem::GetBoundDeliveryStationCount() const
{
	int32 ValidCount = 0;
	for (const TWeakObjectPtr<ADeliveryStation>& WeakStation : BoundStations)
	{
		if (WeakStation.IsValid())
		{
			++ValidCount;
		}
	}

	return ValidCount;
}

void UOrderDeliveryBridgeSubsystem::HandleDeliveryItem(FPrimaryAssetId DeliveredItemId, AActor* SourceStation)
{
	if (!IsAuthorityWorld() || !DeliveredItemId.IsValid())
	{
		return;
	}

	if (UOrderSystem* OrderSystem = GetWorld()->GetSubsystem<UOrderSystem>())
	{
		FDeliveredItemPayload Payload;
		Payload.DeliveredItemId = DeliveredItemId;
		Payload.ServerTimeSeconds = -1.0f;
		OrderSystem->SubmitDeliveryWithContext(Payload, SourceStation);
	}
}

void UOrderDeliveryBridgeSubsystem::HandleActorSpawned(AActor* SpawnedActor)
{
	if (!IsAuthorityWorld() || SpawnedActor == nullptr)
	{
		return;
	}

	if (ADeliveryStation* DeliveryStation = Cast<ADeliveryStation>(SpawnedActor))
	{
		RegisterDeliveryStation(DeliveryStation);
	}
}
