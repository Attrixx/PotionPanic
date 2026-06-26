// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "RangeComponent.generated.h"

class UActorFilter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBestActorChangedDelegate, AActor*, NewBest, AActor*, PreviousBest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBestMatchingActorChangedDelegate, UActorFilter*, Filter, AActor*, NewBest, AActor*, PreviousBest);

/**
 * Capsule-shaped proximity component ranking overlapping actors by a score combining
 * forward-facing dot product and normalized distance. Filters can be tracked via
 * TrackFilter for change notifications. Tune PrimaryComponentTick.TickInterval.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PLAYER_API URangeComponent : public UCapsuleComponent
{
	GENERATED_BODY()

protected:

	URangeComponent();
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	UFUNCTION(BlueprintCallable, Category = "Range Component")
	bool IsActorInRange(AActor* InActor) const;

	/** Returns the highest-scoring actor currently in range, regardless of filter. */
	UFUNCTION(BlueprintCallable, Category = "Range Component")
	AActor* GetBestActor() const { return BestActor.Get(); }

	/**
	 * Returns the best cached actor matching the given filter.
	 * The filter must first be registered via TrackFilter.
	 */
	UFUNCTION(BlueprintCallable, Category = "Range Component")
	AActor* GetBestMatchingActor(UActorFilter* Filter) const;

	/**
	 * Searches the sorted in-range list and returns the first actor matching the given filter.
	 * Does not require prior tracking; use GetBestMatchingActor when the filter is tracked.
	 */
	UFUNCTION(BlueprintCallable, Category = "Range Component")
	AActor* FindBestMatchingActor(UActorFilter* Filter) const;

	/**
	 * Registers a filter for best-actor tracking and change notifications via OnBestMatchingActorChanged.
	 * Reference-counted by Filter identity: each call must be paired with a corresponding UntrackFilter.
	 * The caller keeps ownership of Filter (e.g. via a UPROPERTY) and must keep it alive while tracked.
	 */
	UFUNCTION(BlueprintCallable, Category = "Range Component")
	void TrackFilter(UActorFilter* Filter);

	/** Decrements the tracking ref-count for the given filter. Removes the record when it reaches zero. */
	UFUNCTION(BlueprintCallable, Category = "Range Component")
	void UntrackFilter(UActorFilter* Filter);

	/** Fired when the overall best actor in range changes. */
	UPROPERTY(BlueprintAssignable, Category = "Range Component|Delegate")
	FBestActorChangedDelegate OnBestActorChanged;

	/** Fired when the best actor matching a tracked filter changes. */
	UPROPERTY(BlueprintAssignable, Category = "Range Component|Delegate")
	FBestMatchingActorChangedDelegate OnBestMatchingActorChanged;

private:

	void SortInRangeInfos();
	void NotifyIfBestActorChanged();
	void NotifyIfBestMatchingActorsChanged();

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

	// This array is sorted
	TArray<FInRangeInfo> InRangeInfos;

	struct FFilterRecord
	{
		FFilterRecord(UActorFilter* Filter, AActor* BestActor);

		TWeakObjectPtr<UActorFilter> Filter;
		uint32 NbOccurrences;
		TWeakObjectPtr<AActor> BestActor;
	};

	TArray<FFilterRecord> FilterRecords;

	TWeakObjectPtr<AActor> BestActor;
};
