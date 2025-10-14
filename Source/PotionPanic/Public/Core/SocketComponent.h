#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SocketComponent.generated.h"

class USocketableComponent;

UCLASS(meta = (BlueprintSpawnableComponent))
class USocketComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	// Put a socketable on this socket.
	void Put(USocketableComponent& Socketable);

	// Take the held socketable if any.
	// IsHolding returns false after this method.
	USocketableComponent* Take();

	// Returns true is there is a socketable on this socket.
	bool IsHolding() const;

	// Returns the socketable on this socket
	USocketableComponent* GetHeld() const;

	// Swap held socketables with another socket.
	void Swap(USocketComponent& Other);

private:

	TObjectPtr<USocketableComponent> Held;
};
