#include "Core/FlyingSocket.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include <GameFramework/ProjectileMovementComponent.h>
#include <Components/SphereComponent.h>
#include <Components/AudioComponent.h>
#include <NiagaraComponent.h>
#include <Logging/StructuredLog.h>

AFlyingSocket::AFlyingSocket()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SocketCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Socket Collision"));
	SocketCollision->InitSphereRadius(50.f);
	SocketCollision->SetupAttachment(RootComponent);

	DropOrBreakCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Drop or Break Collision"));
	DropOrBreakCollision->InitSphereRadius(10.f);
	DropOrBreakCollision->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = false;

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->SetupAttachment(RootComponent);
	Niagara->bAutoActivate = false;

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(RootComponent);
	Audio->bAutoActivate = false;

	Socket = CreateDefaultSubobject<USocketComponent>(TEXT("Socket"));
	Socket->SetupAttachment(RootComponent);
}

void AFlyingSocket::BeginPlay()
{
	Super::BeginPlay();

	SocketCollision->OnComponentBeginOverlap.AddDynamic(this, &AFlyingSocket::OnSocketBeginOverlap);
	DropOrBreakCollision->OnComponentBeginOverlap.AddDynamic(this, &AFlyingSocket::OnDropOrBreakBeginOverlap);
}

void AFlyingSocket::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	SocketCollision->OnComponentBeginOverlap.RemoveAll(this);
	DropOrBreakCollision->OnComponentBeginOverlap.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void AFlyingSocket::OnSocketBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TSet Components = OtherActor->GetComponents();
	for (auto* Comp : Components)
	{
		if (auto* OtherSocket = Cast<USocketComponent>(Comp))
		{
			if (OtherSocket->IsHolding())
				continue; // Until we find one that can hold ours

			auto* Socketable = Socket->Take();
			if (Socketable)
				OtherSocket->Put(*Socketable);

			Audio->SetSound(CatchSound);
			Audio->Play();
			break;
		}
	}
}

void AFlyingSocket::OnDropOrBreakBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto* Socketable = Socket->Take())
	{
		// We did not hit a socket
		// TODO: Break item if it is breakable
	}

	Destroy();
}

void AFlyingSocket::IgnoreActor(AActor* ActorToIgnore)
{
	SocketCollision->IgnoreActorWhenMoving(ActorToIgnore, true);
	DropOrBreakCollision->IgnoreActorWhenMoving(ActorToIgnore, true);
}

void AFlyingSocket::Launch(USocketableComponent& Socketable, const FVector& Force)
{

	SocketCollision->IgnoreActorWhenMoving(Socketable.GetOwner(), true);
	DropOrBreakCollision->IgnoreActorWhenMoving(Socketable.GetOwner(), true);
	Socket->Put(Socketable);
	ProjectileMovement->AddForce(Force);
	Niagara->Activate(true);
	Audio->SetSound(LaunchSound);
	Audio->Play();
}
