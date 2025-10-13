#include "Core/FlyingSocket.h"
#include "Core/SocketComponent.h"
#include <GameFramework/ProjectileMovementComponent.h>
#include <Components/SphereComponent.h>
#include <Components/AudioComponent.h>
#include <NiagaraComponent.h>
#include <Logging/StructuredLog.h>

AFlyingSocket::AFlyingSocket()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetCollisionProfileName(TEXT("Projectile"));
	Collision->InitSphereRadius(10.f);
	RootComponent = Collision;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = false;

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	Niagara->SetupAttachment(RootComponent);
	Niagara->bAutoActivate = false;

	Audio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	Audio->SetupAttachment(RootComponent);
	Audio->bAutoActivate = false;

	// Bind de la détection d’impact
	Collision->OnComponentHit.AddDynamic(this, &AFlyingSocket::OnHit);
}

void AFlyingSocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == this)
		return;

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

	if (auto* Socketable = Socket->Take())
	{
		// We did not hit a socket, drop item
	}

	Destroy();
}

void AFlyingSocket::Launch(USocketableComponent& Socketable, const FVector& Direction, float Speed)
{
	Socket->Put(Socketable);
	ProjectileMovement->Velocity = Direction * Speed;
	Niagara->Activate(true);
	Audio->SetSound(LaunchSound);
	Audio->Play();
}
