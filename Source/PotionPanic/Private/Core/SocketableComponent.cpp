#include "Core/SocketableComponent.h"
#include "Core/SocketComponent.h"
#include "DistinguishSystem/DistinguishComponent.h"
#include <Logging/StructuredLog.h>

DEFINE_LOG_CATEGORY_STATIC(MS_SocketableComponent, Log, All);

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

void USocketableComponent::SnapToGround()
{
	FHitResult HitResult;
	FVector CompLoc = GetComponentLocation();
	FVector End = CompLoc - FVector(0, 0, 1000.0f);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CompLoc,
		End,
		ECC_WorldStatic, // ECC_Visibility ?
		Params
	);

	if (!bHit)
	{
		UE_LOGFMT(MS_SocketableComponent, Error, "LineTrace did not hit.");
		return;
	}

	FVector ActorLoc = GetOwner()->GetActorLocation();
	FVector CompToActor = ActorLoc - CompLoc;
	FVector SnapLoc = HitResult.Location + CompToActor;

	GetOwner()->SetActorLocation(SnapLoc);
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
