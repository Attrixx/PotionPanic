#include "Core/SocketableComponent.h"
#include "Core/SocketComponent.h"
#include "DistinguishSystem/DistinguishComponent.h"

void USocketableComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->GetComponents<UDistinguishComponent>(DistinguishComponents);
}

void USocketableComponent::PutOn(USocketComponent& Socket, bool bBroadcastCallback)
{
	Socket.Put(*this, bBroadcastCallback);
}

void USocketableComponent::Take(bool bBroadcastCallback)
{
	if (Holder)
	{
		check(Holder->GetHeld() == this);
		Holder->Take(bBroadcastCallback);
	}
}

bool USocketableComponent::IsHeld() const
{
	return !!Holder;
}

USocketComponent* USocketableComponent::GetHolder() const
{
	return Holder;
}

void USocketableComponent::SetDistinguish(bool bDistinguish)
{
	for (UDistinguishComponent* Comp : DistinguishComponents)
	{
		if (Comp)
		{
			Comp->SetActivate(bDistinguish);
		}
	}
}

bool USocketableComponent::GetDistinguish() const
{
	// TODO : FRANCOIS - Make sure all components are meant to be ACTIVATED
	return DistinguishComponents.Num() > 0 && DistinguishComponents[0] && DistinguishComponents[0]->IsActivated();
}
