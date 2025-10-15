#include "Core/PotionPanicCharacter.h"
#include "Core/CamTargetComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/FlyingSocket.h"
#include "Core/InteractionInterface.h"
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

void APotionPanicCharacter::BeginPlay()
{
	Super::BeginPlay();

	PickupRange->OnComponentBeginOverlap.AddDynamic(this, &APotionPanicCharacter::OnComponentBeginOverlap);
	PickupRange->OnComponentEndOverlap.AddDynamic(this, &APotionPanicCharacter::OnComponentEndOverlap);
}

void APotionPanicCharacter::Tick(float DeltaTime)
{
	SortActorsInRange();
}

void APotionPanicCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	PickupRange->OnComponentBeginOverlap.RemoveAll(this);
	PickupRange->OnComponentEndOverlap.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void APotionPanicCharacter::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this)
		return;

	++ActorsInRange.FindOrAdd(OtherActor);
}

void APotionPanicCharacter::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	int32* CountPtr = ActorsInRange.Find(OtherActor);
	if (!CountPtr)
		return;

	if (--(*CountPtr) <= 0)
	{
		ActorsInRange.Remove(OtherActor);
	}
}

void APotionPanicCharacter::SortActorsInRange()
{
	BestSocket = nullptr;
	BestSocketable = nullptr;
	BestInteractableComponent = nullptr;
	float BestDot = -1.0f;

	TArray<AActor*> InRange;
	ActorsInRange.GetKeys(InRange);
	for (auto* ActorInRange : InRange)
	{
		// TODO: Better picking
		if (auto* OtherSocket = ActorInRange->GetComponentByClass<USocketComponent>())
		{
			if (OtherSocket->IsHolding())
				continue;

			BestSocket = OtherSocket;
		}
		else if (auto* Socketable = ActorInRange->GetComponentByClass<USocketableComponent>())
		{
			BestSocketable = Socketable;
		}

		for (UActorComponent* Component : ActorInRange->GetComponents())
		{
			if (Component->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
			{
				// Get Angle between forward vector and direction to component
				FVector ToActor = (ActorInRange->GetActorLocation() - GetActorLocation()).GetSafeNormal();
				float Dot = FVector::DotProduct(GetActorForwardVector(), ToActor);
				if (Dot > BestDot)
				{
					BestDot = Dot;
					BestInteractableComponent = Component;
				}
			}
		}
	}
}

void APotionPanicCharacter::OnInteract()
{
	if (IsHolding())
	{
		ThrowHeldObject();
	}
	else
	{
		Interact();
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

	FTransform FlyingSocketTransform =
	{
		GetActorRotation(),
		Socket->GetComponentLocation()
	};

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	auto* FlyingSocket = GetWorld()->SpawnActor<AFlyingSocket>(FlyingSocketClass, FlyingSocketTransform, SpawnParameters);
	if (!FlyingSocket)
	{
		UE_LOGFMT(MS_PotionPanicCharacter, Error, "Could not spawn {0}.",
			FlyingSocketClass ? FlyingSocketClass->GetName() : "NULL");
		return;
	}

	FlyingSocket->IgnoreActor(this);
	FlyingSocket->Launch(*Socketable, GetActorForwardVector() * ObjectThrowSpeed);
}

void APotionPanicCharacter::Interact()
{	
	if (BestInteractableComponent == nullptr) return;

	if (IInteractionInterface* Interaction = Cast<IInteractionInterface>(BestInteractableComponent))
	{
		Interaction->Interact(this);
	}
}

void APotionPanicCharacter::DropObject()
{
	auto* Socketable = Socket->Take();
	if (!Socketable)
		return;

	if (BestSocket)
		BestSocket->Put(*Socketable);
}

void APotionPanicCharacter::PickupObject()
{
	if (BestSocketable)
		Socket->Put(*BestSocketable);
}

bool APotionPanicCharacter::IsHolding() const
{
	return Socket->IsHolding();
}
