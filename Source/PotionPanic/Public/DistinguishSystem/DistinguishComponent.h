#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DistinguishComponent.generated.h"

UCLASS(Abstract)
class POTIONPANIC_API UDistinguishComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void SetActivate(bool bActivate);
	bool IsActivated() const { return bIsActivated; }

protected:
	virtual void OnActivate() { checkNoEntry(); }
	virtual void OnDeactivate() { checkNoEntry(); }

private:
	bool bIsActivated = false;
};