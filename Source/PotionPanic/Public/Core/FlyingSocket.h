#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlyingSocket.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class UAudioComponent;
class USocketComponent;
class USocketableComponent;
class USoundBase;

UCLASS(Abstract)
class AFlyingSocket : public AActor
{
	GENERATED_BODY()

private:

	AFlyingSocket();
	void EndPlay(EEndPlayReason::Type) override;

	UFUNCTION() void OnSocketBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnDropOrBreakHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
protected: // Components

	UPROPERTY(VisibleDefaultsOnly, Category = "Flying Socket")
	TObjectPtr<USphereComponent> SocketCollision;

	UPROPERTY(VisibleDefaultsOnly, Category = "Flying Socket")
	TObjectPtr<USphereComponent> DropOrBreakCollision;

	UPROPERTY(VisibleAnywhere, Category = "Flying Socket")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, Category = "Flying Socket")
	TObjectPtr<UNiagaraComponent> NiagaraTrail;

	UPROPERTY(EditAnywhere, Category = "Flying Socket")
	TObjectPtr<UNiagaraSystem> NiagaraHit;

	UPROPERTY(VisibleAnywhere, Category = "Flying Socket")
	TObjectPtr<USocketComponent> Socket;

protected: // Properties

	UPROPERTY(EditAnywhere, Category = "Flying Socket")
	TObjectPtr<USoundBase> LaunchSound;

	UPROPERTY(EditAnywhere, Category = "Flying Socket")
	TObjectPtr<USoundBase> CatchSound;

	UPROPERTY(EditAnywhere, Category = "Flying Socket")
	TObjectPtr<USoundBase> SnapOnSocketSound;

public:

	void IgnoreActor(AActor* ActorToIgnore);
	bool IsIgnored(AActor* ActorToCheck) const;

	void Launch(USocketableComponent& Socketable, const FVector& Force);

private:

	// Actors that are ignored for collisions
	TArray<TObjectPtr<AActor>> IgnoredActors;
};
