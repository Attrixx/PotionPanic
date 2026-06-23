// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "RangeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBestActorChangedDelegate, AActor*, NewBest, AActor*, PreviousBest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBestActorImplementingChangedDelegate, TSubclassOf<UInterface>, Interface, AActor*, NewBest, AActor*, PreviousBest);

/**
 * Capsule-shaped proximity component ranking overlapping actors by a score combining
 * forward-facing dot product and normalized distance. Interfaces can be tracked via
 * TrackInterface for change notifications. Tune PrimaryComponentTick.TickInterval.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLAYER_API URangeComponent : public UCapsuleComponent
{
	GENERATED_BODY()

protected:

	URangeComponent();
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	
	UFUNCTION(Blueprintable)
	bool IsActorInRange(AActor* InActor) const;
	
	/** Returns the highest-scoring actor currently in range, regardless of interface. */
	UFUNCTION(BlueprintCallable)
	AActor* GetBestActor() const { return BestActor.Get(); }

	/**
	 * Returns the best cached actor implementing the given interface.
	 * The interface must first be registered via TrackInterface.
	 */
	UFUNCTION(BlueprintCallable)
	AActor* GetBestActorImplementing(TSubclassOf<UInterface> Interface) const;
	
	/**
	 * Searches the sorted in-range list and returns the first actor implementing the given interface.
	 * Does not require prior tracking; use GetBestActorImplementing when the interface is tracked.
	 */
	UFUNCTION(BlueprintCallable)
	AActor* FindBestActorImplementing(TSubclassOf<UInterface> Interface) const;
	
	/**
	 * Registers an interface for best-actor tracking and change notifications via OnBestActorImplementingChanged.
	 * Reference-counted: each call must be paired with a corresponding UntrackInterface.
	 */
	UFUNCTION(BlueprintCallable)
	void TrackInterface(TSubclassOf<UInterface> Interface);

	/** Decrements the tracking ref-count for the given interface. Removes the record when it reaches zero. */
	UFUNCTION(BlueprintCallable)
	void UntrackInterface(TSubclassOf<UInterface> Interface);

	/** Fired when the overall best actor in range changes. */
	UPROPERTY(BlueprintAssignable)
	FBestActorChangedDelegate OnBestActorChanged;

	/** Fired when the best actor implementing a tracked interface changes. */
	UPROPERTY(BlueprintAssignable)
	FBestActorImplementingChangedDelegate OnBestActorImplementingChanged;

private:

	void SortInRangeInfos();
	void NotifyIfBestActorChanged();
	void NotifyIfBestActorForInterfacesChanged();

	UFUNCTION()
	void Capsule_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                            bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void Capsule_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowDebugBestActors = false;
#endif
	
private:

	struct FInRangeInfo
	{
		FInRangeInfo(AActor* Actor);

		TWeakObjectPtr<AActor> Actor;
		uint32 NbOccurrences;
		float Score;
	};

	TArray<FInRangeInfo> InRangeInfos;

	struct FInterfaceRecord
	{
		FInterfaceRecord(TSubclassOf<UInterface> Interface, AActor* BestActor);

		TSubclassOf<UInterface> Interface;
		uint32 NbOccurrences;
		TWeakObjectPtr<AActor> BestActor;
	};

	TArray<FInterfaceRecord> InterfaceRecords;

	TWeakObjectPtr<AActor> BestActor;
};
