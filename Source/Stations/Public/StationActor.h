#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "GameplayTagContainer.h"
#include "ActivityExecutionState.h"
#include "StationActor.generated.h"

class AItemActor;
class UStationAsset;
class UHolderComponent;
class UActivityExecutor;
class UActivityAsset;

/**
 * AStationActor — Base class for all interactable station actors.
 *
 * =============================================================================
 * DESIGN PHILOSOPHY
 * =============================================================================
 *
 * A station is NOT defined by its visual representation or any hardcoded subclass.
 * It is an abstract gameplay node identified by four structural elements:
 *
 *  1. StationSlot
 *  A GameplayTag that identifies the station's role in the layout system. The layout
 *  subsystem uses this tag to swap stations at runtime without breaking gameplay
 *  references. Think of it as the station's "address" on the map.
 *
 *  2. ItemHolder
 *  A 3D anchor point (UHolderComponent) that holds items to be consumed,
 *  transformed, or produced by the station. This is the spatial contract between
 *  the station and the item system.
 *
 *  3. ActivityExecutor
 *  The runtime executor (UActivityExecutor) responsible for running activities on
 *  this station: QTEs, timed actions, multi-step interactions, etc. The executor
 *  is driven by instructions fetched in the Recipe system; the station itself
 *  holds no activity logic.
 *
 *  4. ImplementedActivities (on UStationAsset)
 *  The set of GameplayTags declaring which activity types this station supports.
 *  Used to fetch instructions available at a given station with the held item.
 *
 * =============================================================================
 * RUNTIME REPLACEABILITY — THE CORE CONSTRAINT
 * =============================================================================
 *
 * Stations must be fully swappable in-place during a game session (layout system).
 * This has a direct architectural consequence:
 *
 *   >> ALL station-specific data lives in UStationAsset.
 *   >> There are NO subclasses of AStationActor.
 *
 * Customization is achieved entirely by assigning a different UStationAsset,
 * not by deriving a new class. This is intentional. Subclassing would break
 * hot-swap because the layout subsystem would need to know the concrete type in
 * advance — the DataAsset model does not have this limitation.
 */
UCLASS(Abstract)
class STATIONS_API AStationActor : public AActor, public IInteractable
{
	GENERATED_BODY()

protected:

	AStationActor();
	void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;

	void Interact_Implementation(AActor* InInstigator) override;
	bool CanInteract_Implementation(AActor* InInstigator) const override;
	
public:
	UFUNCTION(BlueprintCallable)
	void SetStationAsset(UStationAsset* NewStationAsset);

	UFUNCTION(BlueprintCallable)
	UStationAsset* GetStationAsset() const { return StationAsset; }

	UFUNCTION(BlueprintCallable)
	const FGameplayTag& GetStationSlot() const { return StationSlot; }

	UFUNCTION(BlueprintCallable)
	UHolderComponent* GetItemHolder() const { return ItemHolder; }

	UFUNCTION(BlueprintPure)
	UActivityExecutor* GetActivityExecutor() const;

	void TryStartMatchingActivity(AActor* InInstigator);

	/**
	 * Finds the activity matching this station's current state.
	 * @param OutSourceHolder The holder the matched item is currently on, if it's not already
	 *        this station's ItemHolder (i.e. it came from InInstigator). Null otherwise.
	 */
	UActivityAsset* FindMatchingActivity(AActor* InInstigator, UHolderComponent*& OutSourceHolder) const;

	UFUNCTION()
	void Holder_OnCarriableChanged(UHolderComponent* Holder);

	UFUNCTION()
	void Executor_OnExecutionStatusChanged(UActivityExecutor* InExecutor, EActivityExecutionStatus NewStatus);

	UFUNCTION()
	void OnRep_StationAsset();

	void ApplyStationAsset();

private:

	/**
	 * Gets rid of an item the current station asset refuses to store. Ejects it in a game world,
	 * and merely detaches it outside one, where there is no physics to eject into.
	 */
	void DropItemRefusedByAsset();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AItemActor> ItemClass;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_StationAsset)
	TObjectPtr<UStationAsset> StationAsset;

	/**
	 * Identifies this station's role in the layout system.
	 * Can be left to 'None' if this station does not need to be swapped at runtime.
	 */
	UPROPERTY(EditInstanceOnly, meta = (Categories = "StationSlot"))
	FGameplayTag StationSlot;

	/** 3D anchor and ownership point for items processed by this station. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UHolderComponent> ItemHolder;

	/** Drives activity execution (QTEs, interactions, timed sequences) on this station. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UActivityExecutor> Executor;

	/** Spawns and swaps the station's visual representation (AStationVisualActor) from the StationAsset. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UChildActorComponent> VisualActor;

	/** Guards TryStartMatchingActivity against the re-entrancy caused by moving an item onto ItemHolder. */
	bool bStartingActivity = false;
	
	int32 OnActivityGoindSoundHandle;
};
