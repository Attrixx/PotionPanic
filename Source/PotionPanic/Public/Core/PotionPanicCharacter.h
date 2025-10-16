#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PotionPanicCharacter.generated.h"

class USphereComponent;
class AFlyingSocket;
class UCamTargetComponent;
class USocketComponent;
class USocketableComponent;

UCLASS(Abstract)
class POTIONPANIC_API APotionPanicCharacter : public ACharacter
{
	GENERATED_BODY()

	APotionPanicCharacter();

protected:

	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void EndPlay(EEndPlayReason::Type) override;

private:

	UFUNCTION() void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION() void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SortInteractablesInRange();
	void SortSocketablesInRange();
	void SortSocketsInRange();

	float ComputeLocationScore(FVector Location);

	friend class APotionPanicPlayerController;
	void OnInteract();
	void OnCarry();
	void OnDashStart();
	void OnDashEnd();

	void ThrowHeldObject();
	void Interact();
	void DropObject();
	void PickupObject();

	void SetBestSocketable(USocketableComponent* NewBestSocketable);

public:

	bool IsHolding() const;

protected:

	UPROPERTY(VisibleAnywhere, Category = "Potion Panic Character")
	TObjectPtr<USphereComponent> PickupRange;

	UPROPERTY(EditAnywhere, Category = "Potion Panic Character")
	TSubclassOf<AFlyingSocket> FlyingSocketClass;

	UPROPERTY(EditAnywhere, Category = "Potion Panic Character")
	float ObjectThrowSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Potion Panic Character")
	TObjectPtr<USocketComponent> Socket;

private:

	TObjectPtr<UCamTargetComponent> CamTargetComponent;

	TMap<UActorComponent*, int32> InteractableActorsInRange;
	TMap<USocketableComponent*, int32> SocketableComponentsInRange;
	TMap<USocketComponent*, int32> SocketComponentsInRange;

	USocketComponent* BestSocket;
	USocketableComponent* BestSocketable;
	UActorComponent* BestInteractableComponent;

	bool bCanHitDash = true;

};
