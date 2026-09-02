// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameState.h"
#include "WorldData.h"
#include "Rounds/RoundLoader.h"
#include <Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemyGameState, Log, All);

void AAlchemyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAlchemyGameState, SoftWorldData);
	DOREPLIFETIME(AAlchemyGameState, CurrentRound);
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
}

void AAlchemyGameState::CreateOrders()
{
	const FRound* Round = GetCurrentRound();
	check(Round);
	
	float RecipeInterval = Round->Duration / Round->OrderCount;

	RoundOrders.SetNumUninitialized(Round->OrderCount);
	for (int32 i = 0; i < Round->OrderCount; ++i)
	{
		RoundOrders[i] =
		{
			.OrderId = GenOrderId(),
			.Recipe = nullptr, // TODO(francois): probability
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
	// TODO: Start Round
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
