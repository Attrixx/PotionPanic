#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SocketComponent.generated.h"

class USocketableComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHeldChangedCallback, USocketableComponent* OldHeld, USocketableComponent* NewHeld);
DECLARE_MULTICAST_DELEGATE(FOnPutCallback);

UCLASS(meta = (BlueprintSpawnableComponent))
class USocketComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	// Put a socketable on this socket.
	void Put(USocketableComponent& Socketable, bool bBroadcastCallback = true);

	// Take the held socketable if any.
	// IsHolding returns false after this method.
	USocketableComponent* Take(bool bBroadcastCallback = true);

	// Returns true is there is a socketable on this socket.
	bool IsHolding() const;

	// Returns the socketable on this socket
	USocketableComponent* GetHeld() const;

	// Swap held socketables with another socket.
	void Swap(USocketComponent& Other, bool bBroadcastCallback = true);

public:

	FOnHeldChangedCallback OnHeldChanged;
	FOnPutCallback OnPut;

private:

	TObjectPtr<USocketableComponent> Held;
};
