#include "Core/PotionPanicCharacter.h"
#include "Core/PotionPanicPlayerController.h"
#include "Core/CamTargetComponent.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/FlyingSocket.h"
#include "Core/InteractionInterface.h"
#include <Components/SphereComponent.h>
#include <Components/CapsuleComponent.h>
#include <Engine/OverlapResult.h>
#include <Logging/StructuredLog.h>
#include <limits>

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
	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &APotionPanicCharacter::OnHit);
}

void APotionPanicCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (InteractableActorsInRange.Num() >= 2)
	{
		SortInteractablesInRange();
	}

	if (SocketableComponentsInRange.Num() >= 2 && !IsHolding())
	{
		SortSocketablesInRange();
	}

	if (SocketComponentsInRange.Num() >= 2 && IsHolding())
	{
		SortSocketsInRange();
	}
}

void APotionPanicCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	PickupRange->OnComponentBeginOverlap.RemoveAll(this);
	PickupRange->OnComponentEndOverlap.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void APotionPanicCharacter::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->GetOwner() == this)
		return;

	// Only consider actors that implement the interaction interface or are socketable
	// TODO FRANCOIS - For each on all socketables instead
	if (auto* SocketableComponent = OtherActor->GetComponentByClass<USocketableComponent>())
	{
		++SocketableComponentsInRange.FindOrAdd(SocketableComponent);

		if (SocketableComponentsInRange.Num() == 1)
		{
			SetBestSocketable(SocketableComponent);
		}
	}
	// TODO FRANCOIS - Better group of information here
	else if (auto* SocketComponent = OtherActor->GetComponentByClass<USocketComponent>())
	{
		++SocketComponentsInRange.FindOrAdd(SocketComponent);
		if (SocketComponentsInRange.Num() == 1)
		{
			BestSocket = SocketComponent;
		}
	}

	for (auto* Component : OtherActor->GetComponents())
	{
		if (Component->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{
			++InteractableActorsInRange.FindOrAdd(Component);

			if (InteractableActorsInRange.Num() == 1)
			{
				BestInteractableComponent = Component;
				//BestInteractableComponent->SetDistinguish(true);
			}
		}
	}
}

void APotionPanicCharacter::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto* SocketableComponent = OtherActor->GetComponentByClass<USocketableComponent>())
	{
		int32* CountPtr = SocketableComponentsInRange.Find(SocketableComponent);
		if (!CountPtr)
			return;
		if (--(*CountPtr) <= 0)
		{
			SocketableComponentsInRange.Remove(SocketableComponent);
			SetBestSocketable(nullptr);
		}

		if (SocketableComponentsInRange.Num() == 1 && !IsHolding())
		{
			SortSocketablesInRange();
		}
	}
	else if (auto* SocketComponent = OtherActor->GetComponentByClass<USocketComponent>())
	{
		int32* CountPtr = SocketComponentsInRange.Find(SocketComponent);
		if (!CountPtr)
			return;
		if (--(*CountPtr) <= 0)
		{
			SocketComponentsInRange.Remove(SocketComponent);
			BestSocket = nullptr;
		}
		if (SocketComponentsInRange.Num() == 1)
		{
			SortSocketsInRange();
		}
	}

	for (auto* Component : OtherActor->GetComponents())
	{
		if (Component->GetClass()->ImplementsInterface(UInteractionInterface::StaticClass()))
		{
			int32* CountPtr = InteractableActorsInRange.Find(Component);
			if (!CountPtr)
				return;
			if (--(*CountPtr) <= 0)
			{
				InteractableActorsInRange.Remove(Component);
				//if (BestInteractableComponent)
					//BestInteractableComponent->SetDistinguish(false);
				BestInteractableComponent = nullptr;
			}

			if (SocketableComponentsInRange.Num() == 1)
			{
				SortInteractablesInRange();
				//BestInteractableComponent->SetDistinguish(true);
			}
		}
	}
}

void APotionPanicCharacter::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bCanHitDash)
		return;

	if (APotionPanicCharacter* OtherCharacter = Cast<APotionPanicCharacter>(OtherActor))
	{
		if (APotionPanicPlayerController* PC = Cast<APotionPanicPlayerController>(GetController()))
		{
			if (PC->IsDashAvailable()) return;
		}

		bCanHitDash = false;

		bool bCurrentIsHolding = IsHolding();
		bool bTargetIsHolding = OtherCharacter->IsHolding();
		if (bCurrentIsHolding)
		{
			DropObject();
		}
		if (bTargetIsHolding)
		{
			OtherCharacter->DropObject();
		}
	}
}

void APotionPanicCharacter::SortInteractablesInRange()
{
	// Sort Interractable always
	TArray<UActorComponent*> IntarractableInRange;
	InteractableActorsInRange.GetKeys(IntarractableInRange);
	float MaxScore = -std::numeric_limits<float>::infinity();

	for (auto* Component : IntarractableInRange)
	{
		float Score = ComputeLocationScore(Component->GetOwner()->GetActorLocation());
		if (Score > MaxScore)
		{
			MaxScore = Score;
			BestInteractableComponent = Component;
			//BestInteractableComponent->SetDistinguish(true);
		}
	}
}

void APotionPanicCharacter::SortSocketablesInRange()
{
	TArray<USocketableComponent*> SocketableInRange;
	SocketableComponentsInRange.GetKeys(SocketableInRange);
	float MaxScore = std::numeric_limits<float>::lowest();

	for (auto* Socketable : SocketableInRange)
	{
		float Score = ComputeLocationScore(Socketable->GetComponentLocation());
		if (Score >= MaxScore)
		{
			MaxScore = Score;
			SetBestSocketable(Socketable);
		}
	}
}

void APotionPanicCharacter::SortSocketsInRange()
{
	TArray<USocketComponent*> SocketsInRange;
	SocketComponentsInRange.GetKeys(SocketsInRange);
	float MaxScore = std::numeric_limits<float>::lowest();

	for (auto* SocketInRange : SocketsInRange)
	{
		float Score = ComputeLocationScore(SocketInRange->GetComponentLocation());
		if (Score >= MaxScore)
		{
			MaxScore = Score;
			BestSocket = SocketInRange;
		}
	}
}

float APotionPanicCharacter::ComputeLocationScore(FVector Location)
{
	// Get Angle between forward vector and direction to component
	FVector ToActor = (Location - GetActorLocation());
	float Dot = FVector::DotProduct(GetActorForwardVector(), ToActor.GetSafeNormal());
	
	return Dot - ToActor.Length() / PickupRange->GetScaledSphereRadius();
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

void APotionPanicCharacter::OnDashStart()
{
}

void APotionPanicCharacter::OnDashEnd()
{
	bCanHitDash = true;
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
	// TODO FRANCOIS
	auto* Socketable = Socket->Take();
	if (!Socketable)
		return;

	if (BestSocket)
	{
		BestSocket->Put(*Socketable);
	}
	else
	{
		Socketable->SnapToGround();
	}

	SortSocketablesInRange();
}

void APotionPanicCharacter::PickupObject()
{
	if (BestSocketable)
	{
		Socket->Put(*BestSocketable);
		SetBestSocketable(nullptr);
	}

	SortInteractablesInRange();
}

void APotionPanicCharacter::SetBestSocketable(USocketableComponent* NewBestSocketable)
{
	if (BestSocketable == NewBestSocketable)
		return;
	if (BestSocketable)
		BestSocketable->SetDistinguish(false);
	BestSocketable = NewBestSocketable;
	if (BestSocketable)
		BestSocketable->SetDistinguish(true);
}

bool APotionPanicCharacter::IsHolding() const
{
	return Socket->IsHolding();
}
