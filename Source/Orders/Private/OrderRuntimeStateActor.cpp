#include "OrderRuntimeStateActor.h"
#include "Net/UnrealNetwork.h"

AOrderRuntimeStateActor::AOrderRuntimeStateActor()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(5.0f);
	SetMinNetUpdateFrequency(2.0f);
}

void AOrderRuntimeStateActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOrderRuntimeStateActor, TotalScore);
	DOREPLIFETIME(AOrderRuntimeStateActor, ActiveOrders);
	DOREPLIFETIME(AOrderRuntimeStateActor, CompletedOrders);
	DOREPLIFETIME(AOrderRuntimeStateActor, ExpiredOrders);
	DOREPLIFETIME(AOrderRuntimeStateActor, CancelledOrders);
}

void AOrderRuntimeStateActor::SetReplicatedSnapshot(
	int32 InTotalScore,
	const TArray<FReplicatedOrderEntry>& InActiveOrders,
	const TArray<FReplicatedOrderEntry>& InCompletedOrders,
	const TArray<FReplicatedOrderEntry>& InExpiredOrders,
	const TArray<FReplicatedOrderEntry>& InCancelledOrders)
{
	if (!HasAuthority())
	{
		return;
	}

	TotalScore = InTotalScore;
	ActiveOrders = InActiveOrders;
	CompletedOrders = InCompletedOrders;
	ExpiredOrders = InExpiredOrders;
	CancelledOrders = InCancelledOrders;

	OnTotalScoreReplicated.Broadcast(TotalScore);
	OnSnapshotReplicated.Broadcast();
	ForceNetUpdate();
}

void AOrderRuntimeStateActor::OnRep_TotalScore()
{
	OnTotalScoreReplicated.Broadcast(TotalScore);
}

void AOrderRuntimeStateActor::OnRep_ActiveOrders()
{
	OnSnapshotReplicated.Broadcast();
}

void AOrderRuntimeStateActor::OnRep_CompletedOrders()
{
	OnSnapshotReplicated.Broadcast();
}

void AOrderRuntimeStateActor::OnRep_ExpiredOrders()
{
	OnSnapshotReplicated.Broadcast();
}

void AOrderRuntimeStateActor::OnRep_CancelledOrders()
{
	OnSnapshotReplicated.Broadcast();
}
