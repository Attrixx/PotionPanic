// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameState.h"
#include "WorldData.h"
#include "Rounds/RoundLoader.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include <Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemyGameState, Log, All);

namespace
{
	UItemAsset* PickNextOrderItem(TArray<FRoundOrderable>& ItemBag, const TArray<FRoundOrderable>& AllItems, UItemAsset* LastPickedItem)
	{
		if (ItemBag.IsEmpty())
			ItemBag = AllItems;

		float TotalWeight = 0.f;
		for (const FRoundOrderable& Orderable : ItemBag)
			TotalWeight += Orderable.BaseProbability;

		float Roll = FMath::FRandRange(0.f, TotalWeight);
		int32 PickedIndex = ItemBag.Num() - 1;
		for (int32 i = 0; i < ItemBag.Num(); ++i)
		{
			Roll -= ItemBag[i].BaseProbability;
			if (Roll <= 0.f)
			{
				PickedIndex = i;
				break;
			}
		}

		if (ItemBag.Num() > 1 && ItemBag[PickedIndex].Asset.Get() == LastPickedItem)
			PickedIndex = (PickedIndex + 1) % ItemBag.Num();

		UItemAsset* PickedItem = ItemBag[PickedIndex].Asset.Get();
		ItemBag.RemoveAtSwap(PickedIndex);
		return PickedItem;
	}
}

AAlchemyGameState::AAlchemyGameState()
{
	// Only ticks on the server, and only while the current round has orders left to resolve.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AAlchemyGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
		UpdateOrders();
}

void AAlchemyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAlchemyGameState, SoftWorldData);
	DOREPLIFETIME(AAlchemyGameState, CurrentRound);
	DOREPLIFETIME(AAlchemyGameState, RoundStartTime);
	DOREPLIFETIME(AAlchemyGameState, RoundEndTime);
	DOREPLIFETIME(AAlchemyGameState, RoundOrders);
}

void AAlchemyGameState::SetWorldData(const TSoftObjectPtr<UWorldData>& NewWorldData)
{
	if (HasAuthority())
	{
		SoftWorldData = NewWorldData;
		OnRep_SoftWorldData();
	}
}

float AAlchemyGameState::GetRoundTime() const
{
	return GetServerWorldTimeSeconds() - RoundStartTime;
}

float AAlchemyGameState::GetRoundRemainingTime() const
{
	return RoundEndTime - GetServerWorldTimeSeconds();
}

bool AAlchemyGameState::SubmitOrderObject(UObject* DeliveredOrder)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "Item submitted without authority.");
		return false;
	}

	// Only item actors carry the asset identifying what they are, and orders ask for that asset.
	const AItemActor* DeliveredItem = Cast<AItemActor>(DeliveredOrder);
	if (!DeliveredItem)
		return false;

	UItemAsset* DeliveredAsset = DeliveredItem->GetItemAsset();
	if (!DeliveredAsset)
		return false;

	const float RoundTime = GetRoundTime();
	FOrder* Soonest = nullptr;
	double SoonestRemainingTime = 0.0;

	for (FOrder& Order : RoundOrders)
	{
		if (Order.State != EOrderState::Placed || Order.Item != DeliveredAsset)
			continue;

		const double RemainingTime = Order.StartTime + Order.MaxDuration - RoundTime;
		if (!Soonest || RemainingTime < SoonestRemainingTime)
		{
			Soonest = &Order;
			SoonestRemainingTime = RemainingTime;
		}
	}

	if (!Soonest)
		return false;

	SetOrderState(*Soonest, EOrderState::Completed);
	return true;
}

void AAlchemyGameState::OnRep_SoftWorldData()
{
	if (SoftWorldData.IsNull())
	{
		UE_LOGFMT(MS_AlchemyGameState, Error, "Received invalid world data.");
		return;
	}

	SoftWorldData.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateUObject(this, &ThisClass::OnNewWorldDataLoaded));
}

void AAlchemyGameState::OnNewWorldDataLoaded(const FSoftObjectPath& RequestedPath, UObject* InLoadedObject)
{
	if (SoftWorldData.ToSoftObjectPath() != RequestedPath)
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "'{0}' loaded but we were waiting for '{1}'.", SoftWorldData.ToString(), RequestedPath.ToString());
		return;
	}

	auto* NewWorldData = Cast<UWorldData>(InLoadedObject);
	if (!IsValid(NewWorldData))
	{
		UE_LOGFMT(MS_AlchemyGameState, Error, "Loaded invalid world data.");
		return;
	}

	WorldData = NewWorldData;

	if (!HasAuthority())
		return;

	SetCurrentRound(0);
}

void AAlchemyGameState::SetCurrentRound(int32 Index)
{
	if (!WorldData)
	{
		UE_LOGFMT(MS_AlchemyGameState, Error, "No world data to pull rounds from.");
		return;
	}

	const FRound* Round = WorldData->GetRoundAt(Index);
	if (!Round)
	{
		UE_LOGFMT(MS_AlchemyGameState, Error, "No round at index {0} (round count: {1}).", CurrentRound, WorldData->Rounds.Num());
		return;
	}

	CurrentRound = Index;
	FOnRoundAppliedDelegate OnRoundApplied;
	OnRoundApplied.BindDynamic(this, &ThisClass::OnCurrentRoundApplied);
	URoundLoader::LoadAndApplyRound(this, *Round, OnRoundApplied);
	// TODO: Fix load twice without guard
}

const FRound* AAlchemyGameState::GetCurrentRound() const
{
	return WorldData ? WorldData->GetRoundAt(CurrentRound) : nullptr;
}

void AAlchemyGameState::OnCurrentRoundApplied()
{
	CreateOrders();
	StartRound(); // TODO: Wait for everyone ready
}

void AAlchemyGameState::CreateOrders()
{
	const FRound* Round = GetCurrentRound();
	check(Round);
	
	float RecipeInterval = Round->Duration / Round->OrderCount;

	TArray<FRoundOrderable> WeightedItems = Round->Orderables.FilterByPredicate(
		[](const FRoundOrderable& Orderable) { return Orderable.BaseProbability > 0.f; });
	TArray<FRoundOrderable> ItemBag;
	UItemAsset* LastPickedItem = nullptr;

	RoundOrders.SetNumUninitialized(Round->OrderCount);
	for (int32 i = 0; i < Round->OrderCount; ++i)
	{
		LastPickedItem = PickNextOrderItem(ItemBag, WeightedItems, LastPickedItem);

		RoundOrders[i] =
		{
			.OrderId = GenOrderId(),
			.Item = LastPickedItem,
			.State = EOrderState::Pending,
			.StartTime = RecipeInterval * i,
			.MaxDuration = 30.f, // TODO: Parameter this (nb of steps x difficulty?)
		};
	}
}

void AAlchemyGameState::StartRound()
{
	const FRound* Round = GetCurrentRound();
	check(Round);
	
	RoundStartTime = GetServerWorldTimeSeconds();
	RoundEndTime = RoundStartTime + Round->Duration;
	SetActorTickEnabled(true);
}

void AAlchemyGameState::UpdateOrders()
{
	const float RoundTime = GetRoundTime();
	bool bAnyOrderLeft = false;

	for (FOrder& Order : RoundOrders)
	{
		// An order with a tiny MaxDuration may go through both transitions in the same update.
		if (Order.State == EOrderState::Pending && RoundTime >= Order.StartTime)
			SetOrderState(Order, EOrderState::Placed);

		if (Order.State == EOrderState::Placed && RoundTime >= Order.StartTime + Order.MaxDuration)
			SetOrderState(Order, EOrderState::Cancelled);

		bAnyOrderLeft |= Order.State == EOrderState::Pending || Order.State == EOrderState::Placed;
	}

	if (!bAnyOrderLeft || RoundTime >= RoundEndTime)
		[[unlikely]]
		EndRound();
}

void AAlchemyGameState::EndRound()
{
	SetActorTickEnabled(false);

	const TArray<FOrder> EndedOrders = MoveTemp(RoundOrders);
	RoundOrders.Reset();

	for (FOrder Order : EndedOrders)
	{
		Order.State = EOrderState::SystemDeleted;
		OnOrderChanged.Broadcast(Order);
	}
}

void AAlchemyGameState::SetOrderState(FOrder& Order, EOrderState NewState)
{
	Order.State = NewState;
	OnOrderChanged.Broadcast(Order);
}

void AAlchemyGameState::OnRep_RoundOrders(const TArray<FOrder>& OldRoundOrders)
{
	auto FindById = [](const TArray<FOrder>& Orders, uint32 OrderId, int32 IndexHint) -> const FOrder*
	{
		// Indexes should stay stable across updates 
		if (Orders.IsValidIndex(IndexHint) && Orders[IndexHint].OrderId == OrderId)
			return &Orders[IndexHint];
		// Fallback to full search
		return Orders.FindByPredicate([OrderId](const FOrder& Order) { return Order.OrderId == OrderId; });
	};

	for (int32 i = 0; i < RoundOrders.Num(); ++i)
	{
		const FOrder& Order = RoundOrders[i];
		const FOrder* OldOrder = FindById(OldRoundOrders, Order.OrderId, i);

		// Unknown ids count as changed: the order appeared with this update.
		if (!OldOrder || OldOrder->State != Order.State)
			OnOrderChanged.Broadcast(Order);
	}

	for (int32 i = 0; i < OldRoundOrders.Num(); ++i)
	{
		const FOrder& OldOrder = OldRoundOrders[i];
		if (FindById(RoundOrders, OldOrder.OrderId, i))
			continue;

		// The order left the round without an outcome of its own.
		FOrder DeletedOrder = OldOrder;
		DeletedOrder.State = EOrderState::SystemDeleted;
		OnOrderChanged.Broadcast(DeletedOrder);
	}
}
