#include "Core/SocketableComponent.h"
#include "Core/SocketComponent.h"

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
