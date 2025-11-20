#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BinStation.generated.h"

class UStaticMeshComponent;
class USocketComponent;

UCLASS()
class ABinStation : public AActor
{
	GENERATED_BODY()

public:
	ABinStation();

protected:
	virtual void BeginPlay() override;

private:
	void ThrowAway(class USocketableComponent* OldHeld, class USocketableComponent* NewHeld);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USocketComponent> SocketComponent;
};