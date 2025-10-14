#include "Core/PotionPanicCharacter.h"
#include "Core/CamTargetComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/FlyingSocket.h"
#include <Components/SphereComponent.h>
#include <Engine/OverlapResult.h>
#include <Logging/StructuredLog.h>

DEFINE_LOG_CATEGORY_STATIC(MS_PotionPanicCharacter, Log, All);

APotionPanicCharacter::APotionPanicCharacter()
{
	PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
	PickupRange->SetupAttachment(RootComponent);

	CamTargetComponent = CreateDefaultSubobject<UCamTargetComponent>(TEXT("CamTargetComponent"));

	Socket = CreateDefaultSubobject<USocketComponent>(TEXT("Socket"));
	Socket->SetupAttachment(RootComponent);
}

void APotionPanicCharacter::OnInteract()
{
	if (IsHolding())
	{
		ThrowHeldObject();
	}
	else
	{
		// Interract with nearby equipment
	}
}

void APotionPanicCharacter::OnCarry()
{
	if (IsHolding())
	{
		DropObject();
	}
	else
	{
		PickupObject();
	}
}

void APotionPanicCharacter::ThrowHeldObject()
{
	auto* Socketable = Socket->Take();
	if (!Socketable)
		return;

	FVector Location = Socket->GetComponentLocation();
	FRotator Rotation = GetActorRotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* FlyingSocket = GetWorld()->SpawnActor<AFlyingSocket>(FlyingSocketClass, Location, Rotation, SpawnParameters);
	if (!FlyingSocket)
	{
		UE_LOGFMT(MS_PotionPanicCharacter, Error, "Could not spawn FlyingSocket.");
		return;
	}

	FlyingSocket->Launch(*Socketable, GetActorForwardVector() * ObjectThrowSpeed);
}

void APotionPanicCharacter::Interract()
{
	// TODO
}

void APotionPanicCharacter::DropObject()
{
	auto* Socketable = Socket->Take();
	if (!Socketable)
		return;

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		PickupRange->GetComponentLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldDynamic),
		FCollisionShape::MakeSphere(PickupRange->GetScaledSphereRadius()),
		QueryParams
	);

	TArray<USocketComponent*> Sockets;
	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* OverlappedActor = Result.GetActor();
		if (!OverlappedActor)
			continue;

		if (auto* OtherSocket = OverlappedActor->GetComponentByClass<USocketComponent>())
		{
			Sockets.Add(OtherSocket);
		}
	}

	if (Sockets.IsEmpty())
	{
		// TODO: Snap on ground with raycast or smth
		return;
	}

	// TODO: Sort the best socket to drop into (in front for exemple)
	USocketComponent* ChosenSocket = Sockets[0];
	ChosenSocket->Put(*Socketable);
}

void APotionPanicCharacter::PickupObject()
{
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		PickupRange->GetComponentLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldDynamic),
		FCollisionShape::MakeSphere(PickupRange->GetScaledSphereRadius()),
		QueryParams
	);

	TArray<USocketableComponent*> Socketables;
	for (const FOverlapResult& Result : Overlaps)
	{
		AActor* OverlappedActor = Result.GetActor();
		if (!OverlappedActor)
			continue;

		if (auto* Socketable = OverlappedActor->GetComponentByClass<USocketableComponent>())
		{
			Socketables.Add(Socketable);
		}
	}

	if (Socketables.IsEmpty())
		return;

	// TODO: Sort the best socketable to pickup (in front for exemple)
	USocketableComponent* ToGrab = Socketables[0];

	Socket->Put(*ToGrab);
}

bool APotionPanicCharacter::IsHolding() const
{
	return Socket->IsHolding();
}
