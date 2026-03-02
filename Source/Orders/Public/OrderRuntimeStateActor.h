#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "OrderSystem.h"
#include "OrderRuntimeStateActor.generated.h"

USTRUCT(BlueprintType)
struct ORDERS_API FReplicatedOrderEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	FGuid OrderId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	FPrimaryAssetId RequiredOutputItemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	FText OrderName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	float StartTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	float EndTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	float ResolvedTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	EOrderState State = EOrderState::Active;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	int32 AwardedScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Orders|Replication")
	int32 Priority = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOrdersReplicatedScoreChangedEvent, int32, NewTotalScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOrdersReplicatedSnapshotChangedEvent);

UCLASS()
class ORDERS_API AOrderRuntimeStateActor : public AInfo
{
	GENERATED_BODY()

public:
	AOrderRuntimeStateActor();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetReplicatedSnapshot(
		int32 InTotalScore,
		const TArray<FReplicatedOrderEntry>& InActiveOrders,
		const TArray<FReplicatedOrderEntry>& InCompletedOrders,
		const TArray<FReplicatedOrderEntry>& InExpiredOrders,
		const TArray<FReplicatedOrderEntry>& InCancelledOrders);

	UFUNCTION(BlueprintPure, Category = "Orders|Replication")
	int32 GetTotalScore() const { return TotalScore; }

	UFUNCTION(BlueprintPure, Category = "Orders|Replication")
	const TArray<FReplicatedOrderEntry>& GetActiveOrders() const { return ActiveOrders; }

	UFUNCTION(BlueprintPure, Category = "Orders|Replication")
	const TArray<FReplicatedOrderEntry>& GetCompletedOrders() const { return CompletedOrders; }

	UFUNCTION(BlueprintPure, Category = "Orders|Replication")
	const TArray<FReplicatedOrderEntry>& GetExpiredOrders() const { return ExpiredOrders; }

	UFUNCTION(BlueprintPure, Category = "Orders|Replication")
	const TArray<FReplicatedOrderEntry>& GetCancelledOrders() const { return CancelledOrders; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Orders|Replication")
	FOrdersReplicatedScoreChangedEvent OnTotalScoreReplicated;

	UPROPERTY(BlueprintAssignable, Category = "Orders|Replication")
	FOrdersReplicatedSnapshotChangedEvent OnSnapshotReplicated;

private:
	UFUNCTION()
	void OnRep_TotalScore();

	UFUNCTION()
	void OnRep_ActiveOrders();

	UFUNCTION()
	void OnRep_CompletedOrders();

	UFUNCTION()
	void OnRep_ExpiredOrders();

	UFUNCTION()
	void OnRep_CancelledOrders();

private:
	UPROPERTY(ReplicatedUsing = OnRep_TotalScore, VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Replication", meta = (AllowPrivateAccess = "true"))
	int32 TotalScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveOrders, VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Replication", meta = (AllowPrivateAccess = "true"))
	TArray<FReplicatedOrderEntry> ActiveOrders;

	UPROPERTY(ReplicatedUsing = OnRep_CompletedOrders, VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Replication", meta = (AllowPrivateAccess = "true"))
	TArray<FReplicatedOrderEntry> CompletedOrders;

	UPROPERTY(ReplicatedUsing = OnRep_ExpiredOrders, VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Replication", meta = (AllowPrivateAccess = "true"))
	TArray<FReplicatedOrderEntry> ExpiredOrders;

	UPROPERTY(ReplicatedUsing = OnRep_CancelledOrders, VisibleInstanceOnly, BlueprintReadOnly, Category = "Orders|Replication", meta = (AllowPrivateAccess = "true"))
	TArray<FReplicatedOrderEntry> CancelledOrders;
};
