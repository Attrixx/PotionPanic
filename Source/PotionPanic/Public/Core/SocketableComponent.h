#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SocketableComponent.generated.h"

class USocketComponent;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHolderChangedCallback, USocketComponent* OldHolder, USocketComponent* NewHolder);


UCLASS(meta = (BlueprintSpawnableComponent))
class USocketableComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	void BeginPlay() override;

	void PutOn(USocketComponent& Socket, bool bBroadcastCallback = true);
	void Take(bool bBroadcastCallback = true);

	bool IsHeld() const;
	USocketComponent* GetHolder() const;

	FOnHolderChangedCallback OnHolderChanged;
	void SetDistinguish(bool bDistinguish);
	bool GetDistinguish() const;
private:

	friend USocketComponent;
	TObjectPtr<USocketComponent> Holder;

	UPROPERTY()
	TArray<class UDistinguishComponent*> DistinguishComponents;
};
