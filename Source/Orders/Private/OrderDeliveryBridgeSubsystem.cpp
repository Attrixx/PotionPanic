#include "OrderDeliveryBridgeSubsystem.h"
#include "Engine/World.h"
#include "OrderSystem.h"
#include "StationActorBase.h"
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

	for (const TWeakObjectPtr<AStationActorBase>& WeakStation : BoundStations)
	{
		if (AStationActorBase* Station = WeakStation.Get())
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
		if (!It->IsValid() || It->Get()->GetStationKind() != EStationKind::Delivery)
		{
			It.RemoveCurrent();
		}
	}

	for (TActorIterator<AStationActorBase> It(GetWorld()); It; ++It)
	{
		RegisterDeliveryStation(*It);
	}
}

void UOrderDeliveryBridgeSubsystem::RegisterDeliveryStation(AStationActorBase* DeliveryStation)
{
	if (!IsAuthorityWorld()
		|| DeliveryStation == nullptr
		|| DeliveryStation->GetStationKind() != EStationKind::Delivery)
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

void UOrderDeliveryBridgeSubsystem::UnregisterDeliveryStation(AStationActorBase* DeliveryStation)
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
	for (const TWeakObjectPtr<AStationActorBase>& WeakStation : BoundStations)
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
		Payload.SubmissionId = FGuid::NewGuid();
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

	if (AStationActorBase* DeliveryStation = Cast<AStationActorBase>(SpawnedActor))
	{
		RegisterDeliveryStation(DeliveryStation);
	}
}
