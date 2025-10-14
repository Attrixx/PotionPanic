#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SocketableComponent.generated.h"

class USocketComponent;

UCLASS(meta = (BlueprintSpawnableComponent))
class USocketableComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	void PutOn(USocketComponent& Socket);
	void Take();

	bool IsHeld() const;
	USocketComponent* GetHolder() const;

private:

	friend USocketComponent;
	TObjectPtr<USocketComponent> Holder;
};
