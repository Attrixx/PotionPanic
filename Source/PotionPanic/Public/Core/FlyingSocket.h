#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlyingSocket.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UNiagaraComponent;
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

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected: // Components

	UPROPERTY(VisibleDefaultsOnly, Category = "Flying Socket")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Flying Socket")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, Category = "Flying Socket")
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(VisibleAnywhere, Category = "Flying Socket")
	TObjectPtr<UAudioComponent> Audio;

	UPROPERTY(VisibleAnywhere, Category = "Flying Socket")
	TObjectPtr<USocketComponent> Socket;

protected: // Properties

	UPROPERTY(EditAnywhere, Category = "Flying Socket")
	TObjectPtr<USoundBase> LaunchSound;

	UPROPERTY(EditAnywhere, Category = "Flying Socket")
	TObjectPtr<USoundBase> CatchSound;

public:

	void Launch(USocketableComponent& Socketable, const FVector& Direction, float Speed);
};
