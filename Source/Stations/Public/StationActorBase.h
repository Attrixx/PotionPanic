#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "ActivityAsset.h"
#include "Instruction.h"
#include "StationActorBase.generated.h"

class AItemActor;
class APlayerController;

/**
 * Base class for all station actors using the Command Pattern.
 * 
 * Stations execute FInstructions which contain:
 *   - InputItem: What goes in (FPrimaryAssetId)
 *   - OutputItem: What comes out (FPrimaryAssetId)
 *   - Activity: What action to perform (UActivityAsset)
 * 
 * Each station declares which Activities it supports via the Activities array.
 * Execution is validated automatically via CanExecuteActivity().
 * 
 * Supports timed transformations with proximity checking and lifecycle callbacks.
 */
UCLASS()
class STATIONS_API AStationActorBase : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	AStationActorBase();

	void Interact(APlayerController& InInstigator) override;

	UFUNCTION(BlueprintCallable, Category = "Station")
	virtual void Execute(const FInstruction& Instruction);

	UFUNCTION(BlueprintPure, Category = "Station")
	bool CanExecuteActivity(UActivityAsset* Activity) const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Transformation lifecycle - override in derived classes as needed
	virtual void OnTransformationStarted();
	virtual void OnTransformationTick(float DeltaTime);
	virtual void OnTransformationCompleted();
	virtual void OnTransformationCancelled();

	void StartTransformation(float Duration);
	void CancelTransformation();

protected:
	// Configure in Blueprint: which activities this station can perform
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Station")
	TArray<TObjectPtr<UActivityAsset>> Activities;

	// Runtime state
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsTransforming = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	float TransformationProgress = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	float TransformationDuration = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<APlayerController> CurrentPlayer;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	FInstruction CurrentInstruction;
};
