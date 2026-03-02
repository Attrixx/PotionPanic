#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "UtensilActor.h"
#include "Cauldron.generated.h"

/**
 * Portable cauldron item actor.
 * Can consume held ingredients and keep an internal mixed content list.
 * Processing/output rules stay in station systems.
 */
UCLASS()
class ITEMS_API ACauldron : public AUtensilActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	ACauldron();
	virtual void Interact(APlayerController& InInstigator) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Cauldron|Contents")
	bool AddIngredientAssetId(FPrimaryAssetId IngredientAssetId);

	UFUNCTION(BlueprintCallable, Category = "Cauldron|Contents")
	void ClearIngredients();

	UFUNCTION(BlueprintPure, Category = "Cauldron|Contents")
	bool HasIngredients() const { return IngredientContents.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Cauldron|Contents")
	int32 GetIngredientCount() const { return IngredientContents.Num(); }

	UFUNCTION(BlueprintPure, Category = "Cauldron|Contents")
	TArray<FPrimaryAssetId> GetIngredientAssetIds() const { return IngredientContents; }

	UFUNCTION(BlueprintCallable, Category = "Cauldron|Visuals")
	void SetFillRatio(float NewFillRatio);

	UFUNCTION(BlueprintCallable, Category = "Cauldron|Visuals")
	void SetLiquidTint(FLinearColor NewTint);

	UFUNCTION(BlueprintCallable, Category = "Cauldron|Visuals")
	void SetVisualFlags(const TArray<FName>& NewFlags);

	UFUNCTION(BlueprintPure, Category = "Cauldron|Visuals")
	float GetFillRatio() const { return FillRatio; }

	UFUNCTION(BlueprintPure, Category = "Cauldron|Visuals")
	FLinearColor GetLiquidTint() const { return LiquidTint; }

	UFUNCTION(BlueprintPure, Category = "Cauldron|Visuals")
	const TArray<FName>& GetVisualFlags() const { return VisualFlags; }

protected:
	UFUNCTION()
	void OnRep_VisualState();

	UFUNCTION()
	void OnRep_IngredientContents();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cauldron|Visuals")
	void OnVisualStateChangedBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cauldron|Contents")
	void OnContentsChangedBP();

	UPROPERTY(ReplicatedUsing = OnRep_IngredientContents, VisibleInstanceOnly, BlueprintReadOnly, Category = "Cauldron|Contents")
	TArray<FPrimaryAssetId> IngredientContents;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cauldron|Contents", meta = (ClampMin = "1"))
	int32 MaxIngredientVisualCount = 6;

	UPROPERTY(ReplicatedUsing = OnRep_VisualState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Cauldron|Visuals", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FillRatio = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_VisualState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Cauldron|Visuals")
	FLinearColor LiquidTint = FLinearColor::White;

	UPROPERTY(ReplicatedUsing = OnRep_VisualState, VisibleInstanceOnly, BlueprintReadOnly, Category = "Cauldron|Visuals")
	TArray<FName> VisualFlags;

	// TODO (Nath): Replace simple ingredient list with aggregated output computed by the recipe manager.
};
