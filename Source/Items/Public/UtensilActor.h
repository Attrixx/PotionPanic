#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "UtensilActor.generated.h"

/**
 * Base class for tool-like items.
 * Does not encode station/game-flow rules.
 */
UCLASS()
class ITEMS_API AUtensilActor : public AItemActor
{
	GENERATED_BODY()
	
public:
	AUtensilActor();
	virtual void DestroyItem(bool bPlayFeedback = true) override;

	// TODO (Nath): If a special "broken utensil" gameplay loop is added later, keep the original utensil instance persistent.
};
