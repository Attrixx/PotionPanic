#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include <Logging/StructuredLog.h>

DEFINE_LOG_CATEGORY_STATIC(MS_SocketComponent, Log, All);

namespace
{
FString GetOwnerName(UActorComponent* Component)
{
	if (!Component)
		return "Null";

	auto* Owner = Component->GetOwner();
	if (!Owner)
		return "Null";

	return Owner->GetName();
}
}

void USocketComponent::Put(USocketableComponent& Socketable, bool bBroadcastCallback)
{
	USocketableComponent* OldHeld = Held;
	if (Held)
	{
		UE_LOGFMT(MS_SocketComponent, Warning, "Putting {0} onto {1}, but it was already holding {2}!",
			GetOwnerName(&Socketable), GetOwnerName(this), GetOwnerName(Held));

		Take(false);
	}

	USocketComponent* OldHolder = Socketable.Holder;
	if (Socketable.Holder)
	{
		UE_LOGFMT(MS_SocketComponent, Warning, "Putting {0} onto {1}, but it was already held by {2}!",
			GetOwnerName(&Socketable), GetOwnerName(this), GetOwnerName(Socketable.Holder));

		Socketable.Holder->Take(false);
	}

	Held = &Socketable;
	Socketable.Holder = this;

	auto* HeldActor = Held->GetOwner();
	if (!HeldActor)
	{
		UE_LOGFMT(MS_SocketComponent, Error, "Socketable {0} does not have an owner.", Held->GetName());
		return;
	}

	auto HeldLoc = Held->GetComponentLocation();
	auto HolderLoc = GetComponentLocation();
	HeldActor->AddActorWorldOffset(HolderLoc - HeldLoc);
	HeldActor->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);

	if (bBroadcastCallback)
	{
		if (OldHeld)
			OldHeld->OnHolderChanged.Broadcast(this, nullptr);
		if (OldHolder)
			OldHolder->OnHeldChanged.Broadcast(&Socketable, nullptr);

		Socketable.OnHolderChanged.Broadcast(OldHolder, this);
		OnHeldChanged.Broadcast(OldHeld, Held.Get());
	}

	OnPut.Broadcast();
}

USocketableComponent* USocketComponent::Take(bool bBroadcastCallback)
{
	USocketableComponent* RetVal = nullptr;
	if (Held)
	{
		if (auto* HeldActor = Held->GetOwner())
			HeldActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		RetVal = Held;
		Held->Holder = nullptr;
		Held = nullptr;

		if (bBroadcastCallback)
		{
			OnHeldChanged.Broadcast(RetVal, nullptr);
			RetVal->OnHolderChanged.Broadcast(this, nullptr);
		}
	}
	return RetVal;
}

bool USocketComponent::IsHolding() const
{
	return !!Held;
}

USocketableComponent* USocketComponent::GetHeld() const
{
	return Held;
}

void USocketComponent::Swap(USocketComponent& Other, bool bBroadcastCallback)
{
	USocketableComponent* Mine = Take(false);
	USocketableComponent* Theirs = Other.Take(false);
	if (Mine) Other.Put(*Mine, false);
	if (Theirs) Put(*Theirs, false);


	if (bBroadcastCallback)
	{
		if (Mine)
			Mine->OnHolderChanged.Broadcast(this, &Other);
		if (Theirs)
			Theirs->OnHolderChanged.Broadcast(&Other, this);

		Other.OnHeldChanged.Broadcast(Theirs, Mine);
		OnHeldChanged.Broadcast(Mine, Theirs);
	}
}
