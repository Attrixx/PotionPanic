// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemyGameState.h"
#include "WorldData.h"
#include "Rounds/RoundLoader.h"
#include "ActivityExecutor.h"
#include "ItemActor.h"
#include "ItemAsset.h"
#include "StationActor.h"
#include <EngineUtils.h>
#include <Net/UnrealNetwork.h>
#include <GameFramework/PlayerController.h>
#include <TimerManager.h>
#include <Kismet/KismetArrayLibrary.h>

DEFINE_LOG_CATEGORY_STATIC(MS_AlchemyGameState, Log, All);

namespace
{
FRoundOrderable PickNextOrderable(TArray<FRoundOrderable>& ItemBag, const TArray<FRoundOrderable>& AllItems, UItemAsset* LastPickedItem)
{
	if (ItemBag.IsEmpty())
		ItemBag = AllItems;

	// Nothing to draw from: the round has no orderable carrying a weight. Data validation catches
	// it, but a round loaded anyway must not index into an empty bag.
	if (ItemBag.IsEmpty())
		return FRoundOrderable();

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

	FRoundOrderable Picked = ItemBag[PickedIndex];
	ItemBag.RemoveAtSwap(PickedIndex);
	return Picked;
}
}

AAlchemyGameState::AAlchemyGameState()
{
	// Server side only, and only while the current round has orders left to resolve: StartRound
	// enables it and EndRound turns it back off. Clients never enable it, so it stays off there.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AAlchemyGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

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
	DOREPLIFETIME(AAlchemyGameState, Score);
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

int64 AAlchemyGameState::GetScoreToSucceed() const
{
	return WorldData ? WorldData->ScoreToSucceed : 0;
}

bool AAlchemyGameState::DeliverOrder(UItemAsset* ItemAsset)
{
	if (!HasAuthority())
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "Item delivered without authority.");
		return false;
	}

	const float RoundTime = GetRoundTime();
	FItemOrder* Soonest = nullptr;
	double SoonestRemainingTime = 0.0;
	int32 PlacedCount = 0;
	double NextPendingStartTime = 0.0;
	bool bAnyPending = false;

	for (FItemOrder& Order : RoundOrders)
	{
		switch (Order.State)
		{
		case EOrderState::Placed:
		{
			++PlacedCount;
			if (Order.Item != ItemAsset)
				break;

			const double RemainingTime = Order.StartTime + Order.MaxDuration - RoundTime;
			if (!Soonest || RemainingTime < SoonestRemainingTime)
			{
				Soonest = &Order;
				SoonestRemainingTime = RemainingTime;
			}
		}
		break;

		case EOrderState::Pending:
		{
			if (!bAnyPending || Order.StartTime < NextPendingStartTime)
			{
				NextPendingStartTime = Order.StartTime;
				bAnyPending = true;
			}
		}
		break;

		default:
			break;
		}
	}

	if (!Soonest)
		return false;

	// Scored before the state change so the order carries its points into OnOrderChanged.
	Soonest->Score = ScoreForOrder(*Soonest, SoonestRemainingTime);

	UE_LOGFMT(MS_AlchemyGameState, Log, "Order completed by delivering {0}, worth {1} points ({2}s left).",
		ItemAsset ? ItemAsset->ItemName.ToString() : "NULL", Soonest->Score, SoonestRemainingTime);

	AddScore(Soonest->Score);
	SetOrderState(*Soonest, EOrderState::Completed);

	// That was the last thing to work on: pull the tail forward so the players don't idle.
	const FRound* Round = GetCurrentRound();
	if (PlacedCount == 1 && bAnyPending && Round)
		ShiftPendingOrders(NextPendingStartTime - RoundTime - Round->MaxTimeWithoutPlacedOrder);

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

	// This world is the level: its first round starts a fresh run, so the tally starts over too.
	Score = 0;
	LevelCompletedOrders = 0;
	LevelFailedOrders = 0;

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

	// A load still in flight would apply its own layout and start its own round on top of this
	// one, so it is dropped rather than raced.
	CancelPendingRoundStart();

	CurrentRound = Index;
	FOnRoundAppliedDelegate OnRoundApplied;
	OnRoundApplied.BindDynamic(this, &ThisClass::OnCurrentRoundApplied);
	RoundLoader = URoundLoader::LoadAndApplyRound(this, *Round, OnRoundApplied);

	// A round with nothing left to stream is applied from inside the call above.
	if (RoundLoader && !RoundLoader->IsPending())
		RoundLoader = nullptr;
}

void AAlchemyGameState::CancelPendingRoundStart()
{
	if (RoundLoader)
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "Dropping the load of round {0}, still in flight.", CurrentRound);
		RoundLoader->Cancel();
		RoundLoader = nullptr;
	}

	if (RoundStartWaitHandle.IsValid())
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "Dropping the pending start of round {0}.", CurrentRound);
		GetWorldTimerManager().ClearTimer(RoundStartWaitHandle);
	}
}

const FRound* AAlchemyGameState::GetCurrentRound() const
{
	return WorldData ? WorldData->GetRoundAt(CurrentRound) : nullptr;
}

void AAlchemyGameState::OnCurrentRoundApplied()
{
	RoundLoader = nullptr;

	// The round is ready on the server, but starting it now would run its clock while the clients
	// are still streaming the level in. Hold it until they are all there, or until the wait times
	// out: one client that never reports in must not keep the others waiting forever.
	RoundStartWaitDeadline = GetServerWorldTimeSeconds() + MaxRoundStartWaitTime;
	GetWorldTimerManager().SetTimer(RoundStartWaitHandle,
		FTimerDelegate::CreateUObject(this, &ThisClass::TryStartRound),
		RoundStartWaitPollInterval,
		true);

	TryStartRound();
}

bool AAlchemyGameState::AreAllPlayersReady()
{
	UWorld* World = GetWorld();
	if (!World)
		return false;

	// The world data can finish loading before the first controller exists: a round nobody is
	// there to see must not burn its clock either.
	bool bAnyPlayer = false;

	for (auto It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!IsValid(PC))
			continue;

		bAnyPlayer = true;

		// Engine-side signal, the same one CanRestartPlayer gates on. Split-screen child
		// connections carry their own, so every local player counts on its own.
		if (!PC->HasClientLoadedCurrentWorld())
			return false;
	}

	return bAnyPlayer;
}

void AAlchemyGameState::TryStartRound()
{
	const bool bEveryoneReady = AreAllPlayersReady();
	if (!bEveryoneReady && GetServerWorldTimeSeconds() < RoundStartWaitDeadline)
		return;

	if (!bEveryoneReady)
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning,
			"Starting round {0} after {1}s: not every client is ready.", CurrentRound, MaxRoundStartWaitTime);
	}

	GetWorldTimerManager().ClearTimer(RoundStartWaitHandle);
	StartRound();
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
		const FRoundOrderable Picked = PickNextOrderable(ItemBag, WeightedItems, LastPickedItem);
		LastPickedItem = Picked.Asset.Get();

		RoundOrders[i] =
		{
			.OrderId = GenOrderId(),
			.Item = LastPickedItem,
			.State = EOrderState::Pending,
			.StartTime = RecipeInterval * i,
			.MaxDuration = Picked.TimeToComplete,
		};

		// A null item here means the round data asked for something RoundLoader did not resolve.
		UE_LOGFMT(MS_AlchemyGameState, Log, "Created order #{0} for {1}, placed at {2}s.",
			RoundOrders[i].OrderId,
			LastPickedItem ? *LastPickedItem->GetName() : TEXT("NULL"),
			RoundOrders[i].StartTime);
	}
}

void AAlchemyGameState::StartRound()
{
	const FRound* Round = GetCurrentRound();
	check(Round);

	// Created here rather than on round load so the orders reach the clients in the same update as
	// the round timing they are read against: an order list without it resolves against the
	// previous round's start time, and the clients would place and expire every order at once.
	CreateOrders();

	RoundStartTime = GetServerWorldTimeSeconds();
	RoundEndTime = RoundStartTime + Round->Duration;
	SetActorTickEnabled(true);
	Multicast_OnRoundStarted(CurrentRound);
}

void AAlchemyGameState::UpdateOrders()
{
	const float RoundTime = GetRoundTime();
	bool bAnyOrderLeft = false;

	for (FItemOrder& Order : RoundOrders)
	{
		// An order with a tiny MaxDuration may go through both transitions in the same update.
		if (Order.State == EOrderState::Pending && RoundTime >= Order.StartTime)
			SetOrderState(Order, EOrderState::Placed);

		if (Order.State == EOrderState::Placed && RoundTime >= Order.StartTime + Order.MaxDuration)
			SetOrderState(Order, EOrderState::Cancelled);

		bAnyOrderLeft |= Order.State == EOrderState::Pending || Order.State == EOrderState::Placed;
	}
	
	// Ending the round clears RoundOrders, which the clients pick up through OnRep_RoundOrders.
	// Doing it locally there would broadcast every deletion twice.
	if (!HasAuthority())
		return;

	if (!bAnyOrderLeft || GetRoundRemainingTime() <= 0.f)
		[[unlikely]]
			EndRound();
}

void AAlchemyGameState::ShiftPendingOrders(double Shift)
{
	if (Shift <= 0.0)
		return;

	int32 ShiftedCount = 0;
	for (FItemOrder& Order : RoundOrders)
	{
		if (Order.State == EOrderState::Pending)
		{
			Order.StartTime -= Shift;
			++ShiftedCount;
		}
	}

	UE_LOGFMT(MS_AlchemyGameState, Log, "Pulled the {0} pending order(s) {1}s forward.", ShiftedCount, Shift);
}

void AAlchemyGameState::EndRound()
{
	const FRound* Round = GetCurrentRound();
	check(Round);
	
	SetActorTickEnabled(false);
	CancelOngoingStationActivities();
	
	const TArray<FItemOrder> EndedOrders = MoveTemp(RoundOrders);
	RoundOrders.Reset();

	// Anything the players did not deliver counts against them, whether it expired on its own
	// timer or was still open when the round ran out of time.
	int32 CompletedCount = 0;
	for (const FItemOrder& Order : EndedOrders)
	{
		if (Order.State == EOrderState::Completed)
			++CompletedCount;
	}

	LevelCompletedOrders += CompletedCount;
	LevelFailedOrders += EndedOrders.Num() - CompletedCount;

	for (FItemOrder Order : EndedOrders)
	{
		Order.State = EOrderState::SystemDeleted;
		OnOrderChanged.Broadcast(Order);
	}


	Multicast_OnRoundEnded(CurrentRound);

	if (Round->NextRounds.IsEmpty())
	{
		FLevelResult Result;
		Result.Score = Score;
		Result.ScoreToSucceed = GetScoreToSucceed();
		Result.bSucceeded = Result.Score >= Result.ScoreToSucceed;
		Result.CompletedOrders = LevelCompletedOrders;
		Result.FailedOrders = LevelFailedOrders;

		UE_LOGFMT(MS_AlchemyGameState, Log, "Level over: {0} with {1}/{2} points, {3} order(s) delivered and {4} missed.",
			Result.bSucceeded ? TEXT("won") : TEXT("lost"),
			Result.Score, Result.ScoreToSucceed, Result.CompletedOrders, Result.FailedOrders);

		// Listeners will pop a menu to quit, restart, etc.
		Multicast_OnLevelComplete(Result);
		return;
	}
	
	// TODO: Give the choice to players
	int32 Rand = FMath::RandRange(0, Round->NextRounds.Num() - 1);
	int32 NextRound = Round->NextRounds[Rand];
	SetCurrentRound(NextRound);
}

void AAlchemyGameState::CancelOngoingStationActivities()
{
	if (!HasAuthority())
	{
		return;
	}

	// An activity left running past the round is not just untidy: while its QTE step is alive,
	// AAlchemistBase::ShouldBlockGameplayInput() reads UQTEComponent::IsQTERunning() and gates
	// every gameplay input, so the player stays frozen through the whole transition while the
	// station's looping sound carries over into the end-of-round screen.
	//
	// Cancelling from the authority propagates on its own: the executor cancels its current step,
	// which cancels the authority QTE, which tells the owning client to drop its mirror. Executors
	// that are not running ignore the call.
	for (TActorIterator<AStationActor> It(GetWorld()); It; ++It)
	{
		if (UActivityExecutor* Executor = It->GetActivityExecutor())
		{
			Executor->Cancel();
		}
	}
}

void AAlchemyGameState::SetOrderState(FItemOrder& Order, EOrderState NewState)
{
	Order.State = NewState;
	OnOrderChanged.Broadcast(Order);
}

int32 AAlchemyGameState::ScoreForOrder(const FItemOrder& Order, double RemainingTime) const
{
	const FRound* Round = GetCurrentRound();
	if (!Round || Order.MaxDuration <= 0.0)
		return 0;

	const double RemainingRatio = FMath::Clamp(RemainingTime / Order.MaxDuration, 0.0, 1.0);
	return FMath::RoundToInt32(FMath::Lerp<double>(Round->MinOrderScore, Round->MaxOrderScore, RemainingRatio));
}

void AAlchemyGameState::AddScore(int32 Delta)
{
	Score += Delta;
	OnScoreChanged.Broadcast(Score, Delta);
}

void AAlchemyGameState::OnRep_Score(int64 OldScore)
{
	OnScoreChanged.Broadcast(Score, static_cast<int32>(Score - OldScore));
}

void AAlchemyGameState::OnRep_RoundOrders(const TArray<FItemOrder>& OldRoundOrders)
{
	auto FindById = [](const TArray<FItemOrder>& Orders, int32 OrderId, int32 IndexHint) -> const FItemOrder*
	{
		// Indexes should stay stable across updates
		if (Orders.IsValidIndex(IndexHint) && Orders[IndexHint].OrderId == OrderId)
			return &Orders[IndexHint];
		// Fallback to full search
		return Orders.FindByPredicate([OrderId](const FItemOrder& Order) { return Order.OrderId == OrderId; });
	};

	if (UE_LOG_ACTIVE(MS_AlchemyGameState, Verbose))
	{
		FString Summary;
		for (const FItemOrder& Order : RoundOrders)
		{
			// A null item is an unresolved net reference: the order replicated, but this client
			// cannot address the asset it asks for and will not be able to display it.
			Summary += FString::Printf(TEXT("[#%u %s %s] "),
				Order.OrderId,
				*UEnum::GetValueAsString(Order.State),
				Order.Item ? *Order.Item->GetName() : TEXT("UNRESOLVED-ITEM"));
		}

		UE_LOGFMT(MS_AlchemyGameState, Verbose,
			"OnRep_RoundOrders on round {0}: {1} order(s), was {2}, round time {3}. {4}",
			CurrentRound, RoundOrders.Num(), OldRoundOrders.Num(), GetRoundTime(), Summary);
	}

	SetActorTickEnabled(!RoundOrders.IsEmpty());

	for (int32 i = 0; i < RoundOrders.Num(); ++i)
	{
		const FItemOrder& Order = RoundOrders[i];
		const FItemOrder* OldOrder = FindById(OldRoundOrders, Order.OrderId, i);

		// Unknown ids count as changed: the order appeared with this update.
		if (!OldOrder || OldOrder->State != Order.State)
			OnOrderChanged.Broadcast(Order);
	}

	for (int32 i = 0; i < OldRoundOrders.Num(); ++i)
	{
		const FItemOrder& OldOrder = OldRoundOrders[i];
		if (FindById(RoundOrders, OldOrder.OrderId, i))
			continue;

		// The order left the round without an outcome of its own.
		FItemOrder DeletedOrder = OldOrder;
		DeletedOrder.State = EOrderState::SystemDeleted;
		OnOrderChanged.Broadcast(DeletedOrder);
	}
}

void AAlchemyGameState::Multicast_OnRoundStarted_Implementation(int32 RoundIndex)
{
	const FRound* Round = WorldData ? WorldData->GetRoundAt(RoundIndex) : nullptr;
	if (!Round)
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "Cannot broadcast OnRoundStarted: round {0} is not resolvable yet.", RoundIndex);
		return;
	}

	OnRoundStarted.Broadcast(*Round);
}

void AAlchemyGameState::Multicast_OnRoundEnded_Implementation(int32 RoundIndex)
{
	const FRound* Round = WorldData ? WorldData->GetRoundAt(RoundIndex) : nullptr;
	if (!Round)
	{
		UE_LOGFMT(MS_AlchemyGameState, Warning, "Cannot broadcast OnRoundEnded: round {0} is not resolvable yet.", RoundIndex);
		return;
	}

	OnRoundEnded.Broadcast(*Round);
}

void AAlchemyGameState::Multicast_OnLevelComplete_Implementation(const FLevelResult& Result)
{
	OnLevelComplete.Broadcast(Result);
}
