#include "OrderSystem.h"
#include "Engine/World.h"
#include "OrderRuntimeStateActor.h"
#include "OrderDataAsset.h"
#include "TimerManager.h"
#include "EngineUtils.h"

namespace
{
FReplicatedOrderEntry BuildReplicatedEntry(const FOrderRuntime& RuntimeOrder)
{
	FReplicatedOrderEntry Entry;
	Entry.OrderId = RuntimeOrder.OrderId;
	Entry.StartTimeSeconds = RuntimeOrder.StartTimeSeconds;
	Entry.EndTimeSeconds = RuntimeOrder.EndTimeSeconds;
	Entry.ResolvedTimeSeconds = RuntimeOrder.ResolvedTimeSeconds;
	Entry.State = RuntimeOrder.State;
	Entry.AwardedScore = RuntimeOrder.AwardedScore;

	if (RuntimeOrder.Data != nullptr)
	{
		Entry.RequiredOutputItemId = RuntimeOrder.Data->RequiredOutputItemId;
		if (!Entry.RequiredOutputItemId.IsValid())
		{
			for (const FPrimaryAssetId& AltId : RuntimeOrder.Data->AdditionalAcceptedOutputItems)
			{
				if (AltId.IsValid())
				{
					Entry.RequiredOutputItemId = AltId;
					break;
				}
			}
		}
		Entry.OrderName = RuntimeOrder.Data->OrderName;
		Entry.Priority = RuntimeOrder.Data->Priority;
	}

	return Entry;
}
}

void UOrderSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	EnsureReplicatedStateActor();
	ApplyRuntimeConfig();
}

void UOrderSystem::Deinitialize()
{
	if (GetWorld() != nullptr)
	{
		GetWorld()->GetTimerManager().ClearTimer(ExpirationTickTimer);
		GetWorld()->GetTimerManager().ClearTimer(ArrivalTickTimer);
	}
	ReplicatedStateActor = nullptr;

	Super::Deinitialize();
}

bool UOrderSystem::IsAuthorityWorld() const
{
	const UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const ENetMode NetMode = World->GetNetMode();
	return NetMode == NM_Standalone || NetMode == NM_ListenServer || NetMode == NM_DedicatedServer;
}

void UOrderSystem::SetOrderCatalog(const TArray<UOrderDataAsset*>& InOrderCatalog)
{
	OrderCatalog.Reset();
	for (UOrderDataAsset* OrderData : InOrderCatalog)
	{
		if (OrderData != nullptr)
		{
			OrderCatalog.Add(OrderData);
		}
	}

	ApplyRuntimeConfig();
}

FGuid UOrderSystem::SpawnOrder(UOrderDataAsset* OrderData)
{
	if (!IsAuthorityWorld() || OrderData == nullptr)
	{
		return FGuid{};
	}

	FText FailureReason;
	if (!OrderData->IsOrderDefinitionValid(FailureReason))
	{
		return FGuid{};
	}

	const float ServerTime = ResolveServerTime(-1.0f);
	FOrderRuntime NewRuntime;
	NewRuntime.OrderId = FGuid::NewGuid();
	NewRuntime.Data = OrderData;
	NewRuntime.StartTimeSeconds = ServerTime;
	NewRuntime.EndTimeSeconds = ServerTime + FMath::Max(0.1f, OrderData->TimeLimitSeconds);
	NewRuntime.State = EOrderState::Active;

	ActiveOrders.Add(NewRuntime);
	OnOrderSpawned.Broadcast(NewRuntime);
	RefreshReplicatedStateActor();
	return NewRuntime.OrderId;
}

FGuid UOrderSystem::SpawnOrderFromCatalogIndex(int32 CatalogIndex)
{
	if (!OrderCatalog.IsValidIndex(CatalogIndex))
	{
		return FGuid{};
	}

	return SpawnOrder(OrderCatalog[CatalogIndex]);
}

FGuid UOrderSystem::SpawnRandomOrder()
{
	if (OrderCatalog.Num() == 0)
	{
		return FGuid{};
	}

	const int32 RandomIndex = FMath::RandRange(0, OrderCatalog.Num() - 1);
	return SpawnOrder(OrderCatalog[RandomIndex]);
}

void UOrderSystem::CancelOrder(const FGuid& OrderId)
{
	if (!IsAuthorityWorld() || !OrderId.IsValid())
	{
		return;
	}

	for (int32 Index = 0; Index < ActiveOrders.Num(); ++Index)
	{
		if (ActiveOrders[Index].OrderId != OrderId)
		{
			continue;
		}

		TransitionActiveOrderToResolved(
			Index,
			EOrderState::Cancelled,
			ResolveServerTime(-1.0f),
			0,
			CancelledOrders);
		RefreshReplicatedStateActor();
		return;
	}
}

void UOrderSystem::TickOrders(float ServerTimeSeconds)
{
	if (!IsAuthorityWorld())
	{
		return;
	}

	const float ResolvedTime = ResolveServerTime(ServerTimeSeconds);
	ExpireOrders(ResolvedTime);
	RefreshReplicatedStateActor();
}

void UOrderSystem::ClearRuntimeState()
{
	ActiveOrders.Reset();
	CompletedOrders.Reset();
	ExpiredOrders.Reset();
	CancelledOrders.Reset();
	ProcessedSubmissionCache.Reset();
	TotalScore = 0;
	OnTotalScoreChanged.Broadcast(TotalScore);

	ApplyRuntimeConfig();
}

void UOrderSystem::ApplyRuntimeConfig()
{
	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	ExpirationTickIntervalSeconds = FMath::Max(0.05f, ExpirationTickIntervalSeconds);
	ExpirationWarningLeadTimeSeconds = FMath::Max(0.0f, ExpirationWarningLeadTimeSeconds);
	TargetActiveOrderCount = FMath::Max(0, TargetActiveOrderCount);
	MinAutoSpawnIntervalSeconds = FMath::Max(0.1f, MinAutoSpawnIntervalSeconds);
	MaxAutoSpawnIntervalSeconds = FMath::Max(MinAutoSpawnIntervalSeconds, MaxAutoSpawnIntervalSeconds);
	InitialAutoSpawnDelaySeconds = FMath::Max(0.0f, InitialAutoSpawnDelaySeconds);
	OrdersPerArrival = FMath::Max(1, OrdersPerArrival);
	MaxAutoSpawnPerTick = FMath::Max(1, MaxAutoSpawnPerTick);
	ProcessedSubmissionTtlSeconds = FMath::Max(0.0f, ProcessedSubmissionTtlSeconds);
	MaxProcessedSubmissionCacheEntries = FMath::Max(16, MaxProcessedSubmissionCacheEntries);

	UWorld* World = GetWorld();
	check(World != nullptr);

	World->GetTimerManager().ClearTimer(ExpirationTickTimer);
	World->GetTimerManager().SetTimer(
		ExpirationTickTimer,
		this,
		&UOrderSystem::HandleExpirationTick,
		ExpirationTickIntervalSeconds,
		true);

	if (AreAutoOrderArrivalsEnabled())
	{
		if (bSpawnFirstOrderImmediately)
		{
			EnsureTargetActiveOrderCount();
		}

		ScheduleNextOrderArrival(InitialAutoSpawnDelaySeconds);
	}
	else
	{
		World->GetTimerManager().ClearTimer(ArrivalTickTimer);
	}

	RefreshReplicatedStateActor();
}

FOrderSubmissionResult UOrderSystem::SubmitDelivery(const FDeliveredItemPayload& Payload)
{
	return SubmitDeliveryInternal(Payload, nullptr);
}

FOrderSubmissionResult UOrderSystem::SubmitDeliveryWithContext(const FDeliveredItemPayload& Payload, AActor* SourceStation)
{
	return SubmitDeliveryInternal(Payload, SourceStation);
}

FOrderSubmissionResult UOrderSystem::SubmitDeliveryInternal(const FDeliveredItemPayload& Payload, AActor* SourceStation)
{
	FOrderSubmissionResult Result;

	if (!IsAuthorityWorld())
	{
		Result.RejectReason = EOrderSubmissionRejectReason::NotAuthority;
		Result.Reason = FText::FromString(TEXT("SubmitDelivery must run on authority."));
		return Result;
	}

	if (!Payload.DeliveredItemId.IsValid())
	{
		Result.RejectReason = EOrderSubmissionRejectReason::InvalidPayload;
		Result.Reason = FText::FromString(TEXT("Delivered item id is invalid."));
		return Result;
	}

	const float ServerTime = ResolveServerTime(Payload.ServerTimeSeconds);
	FOrderSubmissionResult CachedReplayResult;
	if (TryGetProcessedSubmissionResult(Payload.SubmissionId, CachedReplayResult, ServerTime))
	{
		CachedReplayResult.RejectReason = EOrderSubmissionRejectReason::DuplicateSubmission;
		CachedReplayResult.Reason = FText::FromString(TEXT("Duplicate submission replayed without side effects."));
		return CachedReplayResult;
	}

	auto CacheAndReturn = [this, &Payload, ServerTime](const FOrderSubmissionResult& InResult)
	{
		CacheProcessedSubmissionResult(Payload.SubmissionId, InResult, ServerTime);
		return InResult;
	};

	ExpireOrders(ServerTime);
	RefreshReplicatedStateActor();

	if (ActiveOrders.Num() == 0)
	{
		Result.RejectReason = EOrderSubmissionRejectReason::NoActiveOrders;
		Result.Reason = FText::FromString(TEXT("No active orders."));
		return CacheAndReturn(Result);
	}

	const int32 MatchIndex = FindBestMatchingActiveOrderIndex(Payload.DeliveredItemId, ServerTime);
	if (!ActiveOrders.IsValidIndex(MatchIndex))
	{
		const int32 Penalty = ComputeWrongDeliveryPenalty();
		if (Penalty > 0)
		{
			ApplyScoreDelta(-Penalty);
		}

		Result.bMatched = false;
		Result.ScoreDelta = -Penalty;
		Result.RejectReason = EOrderSubmissionRejectReason::NoMatchingOrder;
		Result.Reason = SourceStation != nullptr
			? FText::Format(
				FText::FromString(TEXT("No active order matches delivered item at station '{0}'.")),
				FText::FromString(SourceStation->GetName()))
			: FText::FromString(TEXT("No active order matches delivered item."));
		OnDeliveryRejected.Broadcast(Payload, Penalty);
		RefreshReplicatedStateActor();
		return CacheAndReturn(Result);
	}

	const FOrderRuntime MatchedOrder = ActiveOrders[MatchIndex];
	const int32 ScoreGain = ComputeCompletionScore(MatchedOrder, ServerTime);
	ApplyScoreDelta(ScoreGain);

	if (!TransitionActiveOrderToResolved(
		MatchIndex,
		EOrderState::Completed,
		ServerTime,
		ScoreGain,
		CompletedOrders))
	{
		Result.RejectReason = EOrderSubmissionRejectReason::InvalidPayload;
		Result.Reason = FText::FromString(TEXT("Order state transition failed during delivery resolution."));
		RefreshReplicatedStateActor();
		return CacheAndReturn(Result);
	}

	Result.bMatched = true;
	Result.MatchedOrderId = MatchedOrder.OrderId;
	Result.ScoreDelta = ScoreGain;
	Result.RejectReason = EOrderSubmissionRejectReason::None;
	Result.Reason = FText::Format(
		FText::FromString(TEXT("Order completed (+{0}).")),
		ScoreGain);

	OnOrderCompleted.Broadcast(MatchedOrder.OrderId, ScoreGain);
	RefreshReplicatedStateActor();
	return CacheAndReturn(Result);
}

TArray<UOrderDataAsset*> UOrderSystem::GetOrderCatalog() const
{
	TArray<UOrderDataAsset*> Raw;
	Raw.Reserve(OrderCatalog.Num());
	for (UOrderDataAsset* Data : OrderCatalog)
	{
		Raw.Add(Data);
	}
	return Raw;
}

float UOrderSystem::ResolveServerTime(float RequestedServerTimeSeconds) const
{
	if (RequestedServerTimeSeconds >= 0.0f)
	{
		return RequestedServerTimeSeconds;
	}

	return GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

bool UOrderSystem::IsOrderInProgressState(EOrderState State)
{
	return State == EOrderState::Active || State == EOrderState::Warning;
}

bool UOrderSystem::CanTransitionOrderState(EOrderState FromState, EOrderState ToState)
{
	if (FromState == EOrderState::Active)
	{
		return ToState == EOrderState::Warning
			|| ToState == EOrderState::Completed
			|| ToState == EOrderState::Expired
			|| ToState == EOrderState::Cancelled;
	}

	if (FromState == EOrderState::Warning)
	{
		return ToState == EOrderState::Completed
			|| ToState == EOrderState::Expired
			|| ToState == EOrderState::Cancelled;
	}

	return false;
}

bool UOrderSystem::TransitionActiveOrderToWarning(int32 ActiveOrderIndex)
{
	if (!ActiveOrders.IsValidIndex(ActiveOrderIndex))
	{
		return false;
	}

	FOrderRuntime& RuntimeOrder = ActiveOrders[ActiveOrderIndex];
	const EOrderState PreviousState = RuntimeOrder.State;
	if (!CanTransitionOrderState(PreviousState, EOrderState::Warning))
	{
		ensureMsgf(
			false,
			TEXT("Invalid order warning transition for order '%s' (%d -> %d)."),
			*RuntimeOrder.OrderId.ToString(EGuidFormats::DigitsWithHyphens),
			static_cast<int32>(PreviousState),
			static_cast<int32>(EOrderState::Warning));
		return false;
	}

	RuntimeOrder.State = EOrderState::Warning;
	OnOrderStateChanged.Broadcast(RuntimeOrder.OrderId, PreviousState, RuntimeOrder.State);
	return true;
}

bool UOrderSystem::TransitionActiveOrderToResolved(
	int32 ActiveOrderIndex,
	EOrderState TargetState,
	float ResolvedTimeSeconds,
	int32 AwardedScore,
	TArray<FOrderRuntime>& OutResolvedOrders)
{
	if (!ActiveOrders.IsValidIndex(ActiveOrderIndex))
	{
		return false;
	}

	const FOrderRuntime SourceOrder = ActiveOrders[ActiveOrderIndex];
	if (!CanTransitionOrderState(SourceOrder.State, TargetState))
	{
		ensureMsgf(
			false,
			TEXT("Invalid order transition for order '%s' (%d -> %d)."),
			*SourceOrder.OrderId.ToString(EGuidFormats::DigitsWithHyphens),
			static_cast<int32>(SourceOrder.State),
			static_cast<int32>(TargetState));
		return false;
	}

	FOrderRuntime ResolvedOrder = SourceOrder;
	ResolvedOrder.State = TargetState;
	ResolvedOrder.ResolvedTimeSeconds = ResolvedTimeSeconds;
	ResolvedOrder.AwardedScore = AwardedScore;

	ActiveOrders.RemoveAt(ActiveOrderIndex);
	OutResolvedOrders.Add(ResolvedOrder);
	OnOrderStateChanged.Broadcast(ResolvedOrder.OrderId, SourceOrder.State, TargetState);
	return true;
}

void UOrderSystem::PruneProcessedSubmissionCache(float ServerTimeSeconds)
{
	if (ProcessedSubmissionCache.Num() == 0)
	{
		return;
	}

	if (ProcessedSubmissionTtlSeconds > 0.0f)
	{
		for (int32 Index = ProcessedSubmissionCache.Num() - 1; Index >= 0; --Index)
		{
			const float AgeSeconds = ServerTimeSeconds - ProcessedSubmissionCache[Index].ProcessedServerTimeSeconds;
			if (AgeSeconds > ProcessedSubmissionTtlSeconds)
			{
				ProcessedSubmissionCache.RemoveAt(Index);
			}
		}
	}

	const int32 MaxEntries = FMath::Max(16, MaxProcessedSubmissionCacheEntries);
	if (ProcessedSubmissionCache.Num() <= MaxEntries)
	{
		return;
	}

	const int32 OverflowCount = ProcessedSubmissionCache.Num() - MaxEntries;
	ProcessedSubmissionCache.RemoveAt(0, OverflowCount);
}

bool UOrderSystem::TryGetProcessedSubmissionResult(const FGuid& SubmissionId, FOrderSubmissionResult& OutResult, float ServerTimeSeconds)
{
	if (!SubmissionId.IsValid())
	{
		return false;
	}

	PruneProcessedSubmissionCache(ServerTimeSeconds);

	for (int32 Index = ProcessedSubmissionCache.Num() - 1; Index >= 0; --Index)
	{
		if (ProcessedSubmissionCache[Index].SubmissionId != SubmissionId)
		{
			continue;
		}

		OutResult = ProcessedSubmissionCache[Index].Result;
		OutResult.bWasReplay = true;
		return true;
	}

	return false;
}

void UOrderSystem::CacheProcessedSubmissionResult(const FGuid& SubmissionId, const FOrderSubmissionResult& Result, float ServerTimeSeconds)
{
	if (!SubmissionId.IsValid())
	{
		return;
	}

	PruneProcessedSubmissionCache(ServerTimeSeconds);

	FOrderSubmissionResult CachedResult = Result;
	CachedResult.bWasReplay = false;

	for (int32 Index = ProcessedSubmissionCache.Num() - 1; Index >= 0; --Index)
	{
		if (ProcessedSubmissionCache[Index].SubmissionId != SubmissionId)
		{
			continue;
		}

		ProcessedSubmissionCache[Index].Result = CachedResult;
		ProcessedSubmissionCache[Index].ProcessedServerTimeSeconds = ServerTimeSeconds;
		return;
	}

	FProcessedSubmissionEntry& Entry = ProcessedSubmissionCache.AddDefaulted_GetRef();
	Entry.SubmissionId = SubmissionId;
	Entry.Result = CachedResult;
	Entry.ProcessedServerTimeSeconds = ServerTimeSeconds;
	PruneProcessedSubmissionCache(ServerTimeSeconds);
}

void UOrderSystem::ExpireOrders(float ServerTimeSeconds)
{
	for (int32 Index = ActiveOrders.Num() - 1; Index >= 0; --Index)
	{
		const FOrderRuntime Candidate = ActiveOrders[Index];
		if (!IsOrderInProgressState(Candidate.State))
		{
			continue;
		}

		if (ServerTimeSeconds >= Candidate.EndTimeSeconds)
		{
			const int32 Penalty = Candidate.Data ? FMath::Max(0, Candidate.Data->ExpirePenalty) : 0;
			if (Penalty > 0)
			{
				ApplyScoreDelta(-Penalty);
			}

			const FGuid ExpiredOrderId = Candidate.OrderId;
			const bool bTransitioned = TransitionActiveOrderToResolved(
				Index,
				EOrderState::Expired,
				ServerTimeSeconds,
				0,
				ExpiredOrders);
			ensureMsgf(bTransitioned, TEXT("Failed transitioning expired order '%s' to Expired state."), *ExpiredOrderId.ToString(EGuidFormats::DigitsWithHyphens));
			OnOrderExpired.Broadcast(ExpiredOrderId, Penalty);
			continue;
		}

		if (Candidate.State != EOrderState::Active || ExpirationWarningLeadTimeSeconds <= 0.0f)
		{
			continue;
		}

		const float RemainingSeconds = Candidate.EndTimeSeconds - ServerTimeSeconds;
		if (RemainingSeconds <= ExpirationWarningLeadTimeSeconds)
		{
			const bool bTransitionedToWarning = TransitionActiveOrderToWarning(Index);
			ensureMsgf(
				bTransitionedToWarning,
				TEXT("Failed transitioning order '%s' to Warning state."),
				*Candidate.OrderId.ToString(EGuidFormats::DigitsWithHyphens));
		}
	}
}

int32 UOrderSystem::ComputeCompletionScore(const FOrderRuntime& Order, float ServerTimeSeconds) const
{
	if (Order.Data == nullptr)
	{
		return 0;
	}

	const float RemainingSeconds = FMath::Max(0.0f, Order.EndTimeSeconds - ServerTimeSeconds);
	const float SafeTimeLimit = FMath::Max(0.1f, Order.Data->TimeLimitSeconds);
	const float TimeBonusRatio = FMath::Clamp(RemainingSeconds / SafeTimeLimit, 0.0f, 1.0f);
	const int32 TimeBonus = FMath::RoundToInt(static_cast<float>(FMath::Max(0, Order.Data->TimeBonusMax)) * TimeBonusRatio);

	return FMath::Max(0, Order.Data->BaseScore) + TimeBonus;
}

int32 UOrderSystem::ComputeWrongDeliveryPenalty() const
{
	const int32 DefaultPenalty = FMath::Max(0, DefaultWrongDeliveryPenalty);

	switch (WrongDeliveryPenaltyPolicy)
	{
	case EWrongDeliveryPenaltyPolicy::FixedDefault:
		return DefaultPenalty;
	case EWrongDeliveryPenaltyPolicy::FirstActiveOrder:
		for (const FOrderRuntime& Order : ActiveOrders)
		{
			if (!IsOrderInProgressState(Order.State) || Order.Data == nullptr)
			{
				continue;
			}

			return FMath::Max(DefaultPenalty, FMath::Max(0, Order.Data->WrongDeliveryPenalty));
		}
		return DefaultPenalty;
	case EWrongDeliveryPenaltyPolicy::MaxActiveOrder:
	default:
	{
		int32 BestPenalty = DefaultPenalty;
		for (const FOrderRuntime& Order : ActiveOrders)
		{
			if (!IsOrderInProgressState(Order.State) || Order.Data == nullptr)
			{
				continue;
			}

			BestPenalty = FMath::Max(BestPenalty, FMath::Max(0, Order.Data->WrongDeliveryPenalty));
		}
		return BestPenalty;
	}
	}
}

int32 UOrderSystem::FindBestMatchingActiveOrderIndex(const FPrimaryAssetId& DeliveredItemId, float ServerTimeSeconds) const
{
	int32 BestIndex = INDEX_NONE;
	float BestEndTime = TNumericLimits<float>::Max();
	int32 BestPriority = TNumericLimits<int32>::Min();

	for (int32 Index = 0; Index < ActiveOrders.Num(); ++Index)
	{
		const FOrderRuntime& Candidate = ActiveOrders[Index];
		if (!IsOrderInProgressState(Candidate.State) || Candidate.Data == nullptr)
		{
			continue;
		}

		if (ServerTimeSeconds >= Candidate.EndTimeSeconds)
		{
			continue;
		}

		FText MatchFailureReason;
		if (!Candidate.Data->MatchesDeliveredItem(DeliveredItemId, MatchFailureReason))
		{
			continue;
		}

		const float CandidateEnd = Candidate.EndTimeSeconds;
		const int32 CandidatePriority = Candidate.Data->Priority;
		const bool bIsBetter =
			BestIndex == INDEX_NONE ||
			CandidateEnd < BestEndTime ||
			(FMath::IsNearlyEqual(CandidateEnd, BestEndTime) && CandidatePriority > BestPriority);

		if (!bIsBetter)
		{
			continue;
		}

		BestIndex = Index;
		BestEndTime = CandidateEnd;
		BestPriority = CandidatePriority;
	}

	return BestIndex;
}

void UOrderSystem::ApplyScoreDelta(int32 DeltaScore)
{
	if (DeltaScore == 0)
	{
		return;
	}

	TotalScore += DeltaScore;
	OnTotalScoreChanged.Broadcast(TotalScore);
}

void UOrderSystem::HandleExpirationTick()
{
	TickOrders(-1.0f);
}

void UOrderSystem::HandleOrderArrivalTick()
{
	if (!IsAuthorityWorld())
	{
		return;
	}

	EnsureTargetActiveOrderCount();
	ScheduleNextOrderArrival();
}

void UOrderSystem::EnsureTargetActiveOrderCount()
{
	if (!IsAuthorityWorld() || !AreAutoOrderArrivalsEnabled() || TargetActiveOrderCount <= 0 || OrderCatalog.Num() == 0)
	{
		return;
	}

	const int32 ArrivalSpawnBudget = FMath::Max(1, OrdersPerArrival);
	const int32 EffectiveSpawnBudget = FMath::Max(1, FMath::Min(ArrivalSpawnBudget, MaxAutoSpawnPerTick));
	int32 SpawnCount = 0;
	while (ActiveOrders.Num() < TargetActiveOrderCount && SpawnCount < EffectiveSpawnBudget)
	{
		if (!SpawnRandomOrder().IsValid())
		{
			break;
		}
		++SpawnCount;
	}
}

void UOrderSystem::ScheduleNextOrderArrival(float OverrideDelaySeconds)
{
	if (!IsAuthorityWorld() || GetWorld() == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	check(World != nullptr);

	World->GetTimerManager().ClearTimer(ArrivalTickTimer);

	if (!AreAutoOrderArrivalsEnabled() || TargetActiveOrderCount <= 0 || OrderCatalog.Num() == 0)
	{
		return;
	}

	const float DelaySeconds = OverrideDelaySeconds >= 0.0f
		? FMath::Max(0.0f, OverrideDelaySeconds)
		: ComputeNextOrderArrivalDelay();

	World->GetTimerManager().SetTimer(
		ArrivalTickTimer,
		this,
		&UOrderSystem::HandleOrderArrivalTick,
		DelaySeconds,
		false);
}

float UOrderSystem::ComputeNextOrderArrivalDelay() const
{
	const float MinDelay = FMath::Max(0.1f, MinAutoSpawnIntervalSeconds);
	const float MaxDelay = FMath::Max(MinDelay, MaxAutoSpawnIntervalSeconds);
	return FMath::FRandRange(MinDelay, MaxDelay);
}

bool UOrderSystem::AreAutoOrderArrivalsEnabled() const
{
	return bAutoOrderArrivalsEnabled;
}

void UOrderSystem::EnsureReplicatedStateActor()
{
	if (!IsAuthorityWorld() || GetWorld() == nullptr || ReplicatedStateActor != nullptr)
	{
		return;
	}

	for (TActorIterator<AOrderRuntimeStateActor> It(GetWorld()); It; ++It)
	{
		ReplicatedStateActor = *It;
		break;
	}

	if (ReplicatedStateActor != nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParameters.Name = TEXT("OrderRuntimeStateActor");
	ReplicatedStateActor = GetWorld()->SpawnActor<AOrderRuntimeStateActor>(AOrderRuntimeStateActor::StaticClass(), FTransform::Identity, SpawnParameters);
}

void UOrderSystem::RefreshReplicatedStateActor()
{
	if (!IsAuthorityWorld())
	{
		return;
	}

	EnsureReplicatedStateActor();
	if (ReplicatedStateActor == nullptr)
	{
		return;
	}

	TArray<FReplicatedOrderEntry> ActiveSnapshot;
	ActiveSnapshot.Reserve(ActiveOrders.Num());
	for (const FOrderRuntime& Runtime : ActiveOrders)
	{
		ActiveSnapshot.Add(BuildReplicatedEntry(Runtime));
	}

	TArray<FReplicatedOrderEntry> CompletedSnapshot;
	CompletedSnapshot.Reserve(CompletedOrders.Num());
	for (const FOrderRuntime& Runtime : CompletedOrders)
	{
		CompletedSnapshot.Add(BuildReplicatedEntry(Runtime));
	}

	TArray<FReplicatedOrderEntry> ExpiredSnapshot;
	ExpiredSnapshot.Reserve(ExpiredOrders.Num());
	for (const FOrderRuntime& Runtime : ExpiredOrders)
	{
		ExpiredSnapshot.Add(BuildReplicatedEntry(Runtime));
	}

	TArray<FReplicatedOrderEntry> CancelledSnapshot;
	CancelledSnapshot.Reserve(CancelledOrders.Num());
	for (const FOrderRuntime& Runtime : CancelledOrders)
	{
		CancelledSnapshot.Add(BuildReplicatedEntry(Runtime));
	}

	ReplicatedStateActor->SetReplicatedSnapshot(
		TotalScore,
		ActiveSnapshot,
		CompletedSnapshot,
		ExpiredSnapshot,
		CancelledSnapshot);
}

AOrderRuntimeStateActor* UOrderSystem::GetReplicatedStateActor()
{
	if (ReplicatedStateActor == nullptr)
	{
		if (IsAuthorityWorld())
		{
			EnsureReplicatedStateActor();
		}
		else if (UWorld* World = GetWorld())
		{
			for (TActorIterator<AOrderRuntimeStateActor> It(World); It; ++It)
			{
				ReplicatedStateActor = *It;
				break;
			}
		}
	}

	return ReplicatedStateActor;
}
