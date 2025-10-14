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

	void SortActorsInRange();

	friend class APotionPanicPlayerController;
	void OnInteract();
	void OnCarry();

	void ThrowHeldObject();
	void Interract();
	void DropObject();
	void PickupObject();

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
	TMap<AActor*, int32> ActorsInRange;

	USocketComponent* BestSocket;
	USocketableComponent* BestSocketable;
};
