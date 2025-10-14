#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PotionPanicCharacter.generated.h"

class USphereComponent;
class AFlyingSocket;
class UCamTargetComponent;
class USocketComponent;

UCLASS(Abstract)
class POTIONPANIC_API APotionPanicCharacter : public ACharacter
{
	GENERATED_BODY()

	APotionPanicCharacter();

private:

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

};
