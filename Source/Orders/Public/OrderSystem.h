#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OrderSystem.generated.h"

class AOrderRuntimeStateActor;
class AActor;
class UOrderDataAsset;
class UWorld;

UENUM(BlueprintType)
enum class EOrderState : uint8
{
	Active UMETA(DisplayName = "Active"),
	Warning UMETA(DisplayName = "Warning"),
	Completed UMETA(DisplayName = "Completed"),
	Expired UMETA(DisplayName = "Expired"),
	Cancelled UMETA(DisplayName = "Cancelled")
};

UENUM(BlueprintType)
enum class EOrderSubmissionRejectReason : uint8
{
	None UMETA(DisplayName = "None"),
	DuplicateSubmission UMETA(DisplayName = "DuplicateSubmission"),
	NotAuthority UMETA(DisplayName = "NotAuthority"),
	InvalidPayload UMETA(DisplayName = "InvalidPayload"),
	NoActiveOrders UMETA(DisplayName = "NoActiveOrders"),
	NoMatchingOrder UMETA(DisplayName = "NoMatchingOrder")
};

UENUM(BlueprintType)
enum class EWrongDeliveryPenaltyPolicy : uint8
{
	FixedDefault UMETA(DisplayName = "FixedDefault"),
	MaxActiveOrder UMETA(DisplayName = "MaxActiveOrder"),
	FirstActiveOrder UMETA(DisplayName = "FirstActiveOrder")
};

USTRUCT(BlueprintType)
struct ORDERS_API FOrderRuntime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Runtime")
	FGuid OrderId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Runtime")
	TObjectPtr<UOrderDataAsset> Data = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Runtime")
	float StartTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Runtime")
	float EndTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Runtime")
	EOrderState State = EOrderState::Active;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Runtime")
	int32 AwardedScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Runtime")
	float ResolvedTimeSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct ORDERS_API FDeliveredItemPayload
{
	GENERATED_BODY()

	/** Optional id used to make delivery submissions idempotent. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Delivery")
	FGuid SubmissionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Delivery")
	FPrimaryAssetId DeliveredItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Delivery")
	float ServerTimeSeconds = -1.0f;
};

USTRUCT(BlueprintType)
struct ORDERS_API FOrderSubmissionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Delivery")
	bool bMatched = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Delivery")
	FGuid MatchedOrderId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Delivery")
	int32 ScoreDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Delivery")
	EOrderSubmissionRejectReason RejectReason = EOrderSubmissionRejectReason::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Delivery")
	FText Reason;

	/** True when this result comes from previously processed submission replay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order|Delivery")
	bool bWasReplay = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrderRuntimeEvent, FOrderRuntime, RuntimeOrder);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOrderStateChangedEvent,
	FGuid,
	OrderId,
	EOrderState,
	PreviousState,
	EOrderState,
	NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOrderCompletedEvent, FGuid, OrderId, int32, ScoreGain);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOrderExpiredEvent, FGuid, OrderId, int32, Penalty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOrderDeliveryRejectedEvent, FDeliveredItemPayload, Payload, int32, Penalty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrderTotalScoreChangedEvent, int32, NewTotalScore);

UCLASS()
class ORDERS_API UOrderSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Orders")
	bool IsAuthorityWorld() const;

	UFUNCTION(BlueprintCallable, Category = "Orders|Runtime")
	void SetOrderCatalog(const TArray<UOrderDataAsset*>& InOrderCatalog);

	UFUNCTION(BlueprintCallable, Category = "Orders|Runtime")
	FGuid SpawnOrder(UOrderDataAsset* OrderData);

	UFUNCTION(BlueprintCallable, Category = "Orders|Runtime")
	FGuid SpawnOrderFromCatalogIndex(int32 CatalogIndex);

	UFUNCTION(BlueprintCallable, Category = "Orders|Runtime")
	FGuid SpawnRandomOrder();

	UFUNCTION(BlueprintCallable, Category = "Orders|Runtime")
	void CancelOrder(const FGuid& OrderId);

	UFUNCTION(BlueprintCallable, Category = "Orders|Runtime")
	void TickOrders(float ServerTimeSeconds = -1.0f);

	UFUNCTION(BlueprintCallable, Category = "Orders|Runtime")
	void ClearRuntimeState();

	UFUNCTION(BlueprintCallable, Category = "Orders|Delivery")
	FOrderSubmissionResult SubmitDelivery(const FDeliveredItemPayload& Payload);

	UFUNCTION(BlueprintCallable, Category = "Orders|Delivery")
	FOrderSubmissionResult SubmitDeliveryWithContext(const FDeliveredItemPayload& Payload, AActor* SourceStation);

	UFUNCTION(BlueprintPure, Category = "Orders|Runtime")
	TArray<UOrderDataAsset*> GetOrderCatalog() const;

	UFUNCTION(BlueprintPure, Category = "Orders|Runtime")
	TArray<FOrderRuntime> GetActiveOrders() const { return ActiveOrders; }

	UFUNCTION(BlueprintPure, Category = "Orders|Runtime")
	TArray<FOrderRuntime> GetCompletedOrders() const { return CompletedOrders; }

	UFUNCTION(BlueprintPure, Category = "Orders|Runtime")
	TArray<FOrderRuntime> GetExpiredOrders() const { return ExpiredOrders; }

	UFUNCTION(BlueprintPure, Category = "Orders|Runtime")
	TArray<FOrderRuntime> GetCancelledOrders() const { return CancelledOrders; }

	UFUNCTION(BlueprintPure, Category = "Orders|Score")
	int32 GetTotalScore() const { return TotalScore; }

	UFUNCTION(BlueprintPure, Category = "Orders|Replication")
	AOrderRuntimeStateActor* GetReplicatedStateActor();

public:
	UPROPERTY(BlueprintAssignable, Category = "Orders|Events")
	FOrderRuntimeEvent OnOrderSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Orders|Events")
	FOrderStateChangedEvent OnOrderStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Orders|Events")
	FOrderCompletedEvent OnOrderCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Orders|Events")
	FOrderExpiredEvent OnOrderExpired;

	UPROPERTY(BlueprintAssignable, Category = "Orders|Events")
	FOrderDeliveryRejectedEvent OnDeliveryRejected;

	UPROPERTY(BlueprintAssignable, Category = "Orders|Events")
	FOrderTotalScoreChangedEvent OnTotalScoreChanged;

private:
	struct FProcessedSubmissionEntry
	{
		FGuid SubmissionId;
		FOrderSubmissionResult Result;
		float ProcessedServerTimeSeconds = 0.0f;
	};

	float ResolveServerTime(float RequestedServerTimeSeconds) const;
	static bool IsOrderInProgressState(EOrderState State);
	static bool CanTransitionOrderState(EOrderState FromState, EOrderState ToState);
	bool TransitionActiveOrderToWarning(int32 ActiveOrderIndex);
	bool TransitionActiveOrderToResolved(
		int32 ActiveOrderIndex,
		EOrderState TargetState,
		float ResolvedTimeSeconds,
		int32 AwardedScore,
		TArray<FOrderRuntime>& OutResolvedOrders);
	void ExpireOrders(float ServerTimeSeconds);
	int32 ComputeCompletionScore(const FOrderRuntime& Order, float ServerTimeSeconds) const;
	int32 ComputeWrongDeliveryPenalty() const;
	int32 FindBestMatchingActiveOrderIndex(const FPrimaryAssetId& DeliveredItemId, float ServerTimeSeconds) const;
	void PruneProcessedSubmissionCache(float ServerTimeSeconds);
	bool TryGetProcessedSubmissionResult(const FGuid& SubmissionId, FOrderSubmissionResult& OutResult, float ServerTimeSeconds);
	void CacheProcessedSubmissionResult(const FGuid& SubmissionId, const FOrderSubmissionResult& Result, float ServerTimeSeconds);
	FOrderSubmissionResult SubmitDeliveryInternal(const FDeliveredItemPayload& Payload, AActor* SourceStation);
	void ApplyScoreDelta(int32 DeltaScore);
	void HandleExpirationTick();
	void HandleOrderArrivalTick();
	void EnsureTargetActiveOrderCount();
	void ScheduleNextOrderArrival(float OverrideDelaySeconds = -1.0f);
	float ComputeNextOrderArrivalDelay() const;
	bool AreAutoOrderArrivalsEnabled() const;
	void SanitizeRuntimeConfigValues();
	void ConfigureExpirationTimer(UWorld& World);
	void ConfigureArrivalTimer(UWorld& World);
	bool TryGetReplaySubmissionResult(const FDeliveredItemPayload& Payload, float ServerTimeSeconds, FOrderSubmissionResult& OutReplayResult);
	FOrderSubmissionResult FinalizeSubmissionResult(const FGuid& SubmissionId, const FOrderSubmissionResult& Result, float ServerTimeSeconds);
	FOrderSubmissionResult BuildNoActiveOrdersResult() const;
	FOrderSubmissionResult BuildCompletedOrderResult(const FOrderRuntime& MatchedOrder, int32 ScoreGain) const;
	FOrderSubmissionResult BuildNoMatchingOrderResult(AActor* SourceStation, int32 Penalty) const;
	void ApplyRuntimeConfig();
	void EnsureReplicatedStateActor();
	void RefreshReplicatedStateActor();

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UOrderDataAsset>> OrderCatalog;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<FOrderRuntime> ActiveOrders;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<FOrderRuntime> CompletedOrders;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<FOrderRuntime> ExpiredOrders;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Runtime", meta = (AllowPrivateAccess = "true"))
	TArray<FOrderRuntime> CancelledOrders;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Score", meta = (AllowPrivateAccess = "true"))
	int32 TotalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 DefaultWrongDeliveryPenalty = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (AllowPrivateAccess = "true"))
	EWrongDeliveryPenaltyPolicy WrongDeliveryPenaltyPolicy = EWrongDeliveryPenaltyPolicy::MaxActiveOrder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "0.05", AllowPrivateAccess = "true"))
	float ExpirationTickIntervalSeconds = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ExpirationWarningLeadTimeSeconds = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (AllowPrivateAccess = "true"))
	bool bAutoOrderArrivalsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "0", AllowPrivateAccess = "true"))
	int32 TargetActiveOrderCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float MinAutoSpawnIntervalSeconds = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "0.1", AllowPrivateAccess = "true"))
	float MaxAutoSpawnIntervalSeconds = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float InitialAutoSpawnDelaySeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (AllowPrivateAccess = "true"))
	bool bSpawnFirstOrderImmediately = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
	int32 OrdersPerArrival = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Config", meta = (ClampMin = "1", AllowPrivateAccess = "true"))
	int32 MaxAutoSpawnPerTick = 3;

	FTimerHandle ExpirationTickTimer;
	FTimerHandle ArrivalTickTimer;

	UPROPERTY(Transient)
	TObjectPtr<AOrderRuntimeStateActor> ReplicatedStateActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Delivery", meta = (ClampMin = "0.0", AllowPrivateAccess = "true"))
	float ProcessedSubmissionTtlSeconds = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orders|Delivery", meta = (ClampMin = "16", AllowPrivateAccess = "true"))
	int32 MaxProcessedSubmissionCacheEntries = 128;

	TArray<FProcessedSubmissionEntry> ProcessedSubmissionCache;
};
