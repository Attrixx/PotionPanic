#pragma once

#include "CoreMinimal.h"
#include "UtensilActor.h"
#include "Interactable.h"
#include "Cauldron.generated.h"

class UItemAsset;

/**
 * Cauldron actor that acts as a container for ingredients.
 * Can be placed on WoodFireStation to cook contents.
 */
UCLASS()
class ITEMS_API ACauldron : public AUtensilActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	ACauldron();

	// IInteractable Interface: Start adding ingredients or pickup management
	virtual void Interact(APlayerController& InInstigator) override;

	 /** Adds an ingredient to the cauldron. */
	UFUNCTION(BlueprintCallable, Category = "Cauldron")
	void AddIngredient(UItemAsset* Ingredient);

	/** Clears all contents (e.g. after successful cooking). */
	UFUNCTION(BlueprintCallable, Category = "Cauldron")
	void EmptyCauldron();

	UFUNCTION(BlueprintPure, Category = "Cauldron")
	const TArray<UItemAsset*>& GetCurrentIngredients() const { return CurrentIngredients; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Cauldron")
	TArray<UItemAsset*> CurrentIngredients;

	// TODO (Nath): Add visual representation of contents (soup color, meshes)
};
