#pragma once

#include "CoreMinimal.h"
#include "ItemActor.h"
#include "UtensilActor.generated.h"

/**
 * Base class for indestructible tools (Cauldron, etc.).
 * Cannot be destroyed by TrashStation.
 */
UCLASS()
class ITEMS_API AUtensilActor : public AItemActor
{
	GENERATED_BODY()
	
public:
	AUtensilActor();
};
