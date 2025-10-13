#include "Core/SocketableComponent.h"
#include "Core/SocketComponent.h"

void USocketableComponent::PutOn(USocketComponent& Socket)
{
	Socket.Put(*this);
}

void USocketableComponent::Take()
{
	if (Holder)
	{
		check(Holder->GetHeld() == this);
		Holder->Take();
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
