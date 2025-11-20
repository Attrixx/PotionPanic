#include "Core/FlyingSocket.h"
#include "Core/SocketComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/PotionPanicCharacter.h"
#include "RecipeSystem/StationComponent.h"
#include <GameFramework/ProjectileMovementComponent.h>
#include <Kismet/GameplayStatics.h>
#include <Components/SphereComponent.h>
#include <Components/AudioComponent.h>
#include <NiagaraComponent.h>
#include "NiagaraFunctionLibrary.h"
#include <Logging/StructuredLog.h>

AFlyingSocket::AFlyingSocket()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	SocketCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Socket Collision"));
	SocketCollision->SetCollisionProfileName("OverlapAll", false);
	SocketCollision->InitSphereRadius(50.f);
	SocketCollision->SetGenerateOverlapEvents(true);
	SocketCollision->SetupAttachment(RootComponent);

	DropOrBreakCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Drop or Break Collision"));
	DropOrBreakCollision->SetCollisionProfileName("BlockAll", false);
	DropOrBreakCollision->SetGenerateOverlapEvents(false);
	DropOrBreakCollision->SetSimulatePhysics(true);
	DropOrBreakCollision->SetEnableGravity(true);
	DropOrBreakCollision->SetNotifyRigidBodyCollision(true);
	DropOrBreakCollision->InitSphereRadius(10.f);
	DropOrBreakCollision->SetupAttachment(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bRotationFollowsVelocity = false;

	NiagaraTrail = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara"));
	NiagaraTrail->SetupAttachment(RootComponent);
	NiagaraTrail->bAutoActivate = false;

	Socket = CreateDefaultSubobject<USocketComponent>(TEXT("Socket"));
	Socket->SetupAttachment(RootComponent);
}

void AFlyingSocket::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	SocketCollision->OnComponentBeginOverlap.RemoveAll(this);
	DropOrBreakCollision->OnComponentHit.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void AFlyingSocket::OnSocketBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsIgnored(OtherActor))
		return;

	TObjectPtr<USocketComponent> TargetSocket = nullptr;
	bool bTargetIsStation = false;

	TSet Components = OtherActor->GetComponents();
	for (auto* Comp : Components)
	{
		if (auto* OtherSocket = Cast<USocketComponent>(Comp))
		{
			if (OtherSocket->IsHolding())
				continue; // Until we find one that can hold ours

			TargetSocket = OtherSocket;
		}
		else if (UStationComponent* Station = Cast<UStationComponent>(Comp))
		{
			bTargetIsStation = true;
		}
	}

	if (TargetSocket == nullptr) return;
	bool bTargetIsCharacter = Cast<APotionPanicCharacter>(OtherActor) != nullptr;

	auto* Socketable = Socket->Take();
	if (Socketable)
		TargetSocket->Put(*Socketable);


	if (bTargetIsCharacter)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CatchSound, TargetSocket->GetComponentLocation(), 1.f, 2.f);
	}
	else if (bTargetIsStation)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SnapOnSocketSound, TargetSocket->GetComponentLocation());
	}
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, NiagaraHit, TargetSocket->GetComponentLocation(), FRotator::ZeroRotator, FVector(0.5f), true, true, ENCPoolMethod::AutoRelease);
}

void AFlyingSocket::OnDropOrBreakHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (IsIgnored(OtherActor))
		return;

	// We did not hit a socket
	if (auto* Socketable = Socket->Take())
	{
		if (false /* TODO: bIsBreakable */)
		{
			// TODO: Break item if it is breakable
		}
		else
		{
			Socketable->SnapToGround();
		}
	}

	Destroy();
}

void AFlyingSocket::IgnoreActor(AActor* ActorToIgnore)
{
	IgnoredActors.RemoveSwap(nullptr);
	IgnoredActors.AddUnique(ActorToIgnore);
}

bool AFlyingSocket::IsIgnored(AActor* ActorToCheck) const
{
	for (auto Ignored : IgnoredActors)
		if (ActorToCheck == Ignored)
			return true;
	return false;
}

void AFlyingSocket::Launch(USocketableComponent& Socketable, const FVector& Force)
{
	IgnoreActor(this);
	IgnoreActor(Socketable.GetOwner());

	Socket->Put(Socketable);
	ProjectileMovement->Velocity = Force;
	NiagaraTrail->Activate(true);

	SocketCollision->OnComponentBeginOverlap.AddDynamic(this, &AFlyingSocket::OnSocketBeginOverlap);
	DropOrBreakCollision->OnComponentHit.AddDynamic(this, &AFlyingSocket::OnDropOrBreakHit);
}
