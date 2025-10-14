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

void USocketComponent::Put(USocketableComponent& Socketable)
{
	if (Held)
	{
		UE_LOGFMT(MS_SocketComponent, Warning, "Putting {0} onto {1}, but it is already holding {2}!",
			GetOwnerName(&Socketable), GetOwnerName(this), GetOwnerName(Held));

		Take();
	}

	if (Socketable.Holder)
	{
		UE_LOGFMT(MS_SocketComponent, Warning, "Putting {0} onto {1}, but it was already held by {2}!",
			GetOwnerName(&Socketable), GetOwnerName(this), GetOwnerName(Socketable.Holder));

		Socketable.Holder->Take();
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
}

USocketableComponent* USocketComponent::Take()
{
	USocketableComponent* RetVal = nullptr;
	if (Held)
	{
		if (auto* HeldActor = Held->GetOwner())
			HeldActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		RetVal = Held;
		Held->Holder = nullptr;
		Held = nullptr;
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

void USocketComponent::Swap(USocketComponent& Other)
{
	USocketableComponent* Mine = Take();
	USocketableComponent* Theirs = Other.Take();
	if (Mine) Other.Put(*Mine);
	if (Theirs) Put(*Theirs);
}
