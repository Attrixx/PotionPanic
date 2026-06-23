// Fill out your copyright notice in the Description page of Project Settings.

#include "RangeComponent.h"
#include "ActorFilters/ActorFilter.h"

DEFINE_LOG_CATEGORY_STATIC(MS_RangeComponent, Log, All);

URangeComponent::URangeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;

	SetGenerateOverlapEvents(true);
	// the following delegate is part of our (inherited) members, binding here and never unbinding is fine
	OnComponentBeginOverlap.AddDynamic(this, &ThisClass::Capsule_OnBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &ThisClass::Capsule_OnEndOverlap);
}

void URangeComponent::OnRegister()
{
	// Force the sensor collision setup HERE (not only in the constructor): a Blueprint owning this
	// component serializes its own collision profile, which would override constructor values.
	// OnRegister runs after those Blueprint values are applied, so this is guaranteed to win.
	// This is a pure detection volume: it must OVERLAP every collidable actor regardless of that
	// actor's own collision (stations block the player, so the capsule must be the one responding
	// Overlap, otherwise the pair resolves to a blocking hit and no overlap event ever fires).
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionObjectType(ECC_WorldDynamic);
	SetCollisionResponseToAllChannels(ECR_Overlap);
	SetGenerateOverlapEvents(true);

	Super::OnRegister();
}

void URangeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SortInRangeInfos();
	NotifyIfBestActorChanged();
	NotifyIfBestMatchingActorsChanged();

#if WITH_EDITOR
	if (bShowDebugBestActors)
	{
		const AActor* Owner = GetOwner();
		if (!GEngine || !Owner) return;

		const float DisplayTime = PrimaryComponentTick.TickInterval * 2.f;
		const auto ShowDebug = [Owner, DisplayTime](const FStringView Type, const UObjectBaseUtility* Best)
		{
			const int32 MsgId = int32(HashCombine(Owner->GetActorGuid().A, GetTypeHash(Type)));
			GEngine->AddOnScreenDebugMessage(
				MsgId,
				DisplayTime,
				FColor::Cyan,
				FString::Format(
					TEXT("{0} Best {1}: {2}"),
					FStringFormatOrderedArguments{
						Owner->GetName(),
						Type,
						GetNameSafe(Best)
					}));
		};

		ShowDebug(TEXT("Actor"), BestActor.Get());
		for (const FFilterRecord& Record : FilterRecords)
		{
			ShowDebug(GetNameSafe(Record.Filter.Get()), Record.BestActor.Get());
		}
	}
#endif
}

bool URangeComponent::IsActorInRange(AActor* InActor) const
{
	for (const FInRangeInfo& InRange : InRangeInfos)
	{
		if (InRange.Actor == InActor)
			return true;
	}
	return false;
}

AActor* URangeComponent::GetBestMatchingActor(UActorFilter* Filter) const
{
	for (const FFilterRecord& Record : FilterRecords)
	{
		if (Record.Filter == Filter)
			return Record.BestActor.Get();
	}

	UE_LOGFMT(MS_RangeComponent, Error, "GetBestActorForFilter: {0} is not tracked. Call TrackFilter first.", GetNameSafe(Filter));
	return nullptr;
}

AActor* URangeComponent::FindBestMatchingActor(UActorFilter* Filter) const
{
	// There should never be more than a few actors in range if the capsule is 
	// set up correctly, iterating is the best option.

	if (!Filter) return nullptr;
	for (const FInRangeInfo& Info : InRangeInfos)
	{
		if (AActor* InfoActor = Info.Actor.Get())
		{
			if (Filter->Matches(InfoActor))
			{
				return InfoActor;
			}
		}
	}
	return nullptr;
}

void URangeComponent::TrackFilter(UActorFilter* Filter)
{
	if (!Filter) return;
	for (FFilterRecord& Record : FilterRecords)
	{
		if (Record.Filter == Filter)
		{
			++Record.NbOccurrences;
			return;
		}
	}
	FilterRecords.Emplace(Filter, FindBestMatchingActor(Filter));
}

void URangeComponent::UntrackFilter(UActorFilter* Filter)
{
	for (auto It = FilterRecords.CreateIterator(); It; ++It)
	{
		if (It->Filter == Filter)
		{
			if (--It->NbOccurrences == 0)
			{
				It.RemoveCurrentSwap();
			}
			break;
		}
	}
}

void URangeComponent::SortInRangeInfos()
{
	if (InRangeInfos.IsEmpty())
		return;

	const float MyCapsuleRadius = FMath::Max(GetScaledCapsuleRadius(), UE_KINDA_SMALL_NUMBER); // Avoid div by 0
	const FVector MyLocation = GetComponentLocation();
	const FVector MyForward = GetComponentQuat().GetForwardVector();

	InRangeInfos.RemoveAll([&](FInRangeInfo& InRangeInfo)
	{
		if (!InRangeInfo.Actor.IsValid())
			return true;

		FVector ToActor = InRangeInfo.Actor->GetActorLocation() - MyLocation;
		float Dot = FVector::DotProduct(MyForward, ToActor.GetSafeNormal());
		float DistToActor = ToActor.Length();
		InRangeInfo.Score = Dot - DistToActor / MyCapsuleRadius;
		return false;
	});

	InRangeInfos.Sort([](const FInRangeInfo& Left, const FInRangeInfo& Right)
	{
		// The best (higher) score should be first in the array
		return Left.Score > Right.Score;
	});
}

void URangeComponent::NotifyIfBestActorChanged()
{
	AActor* OldBest = BestActor.Get();
	AActor* NewBest = nullptr;
	if (InRangeInfos.Num() > 0)
	{
		NewBest = InRangeInfos[0].Actor.Get();
	}

	if (NewBest != OldBest)
	{
		BestActor = NewBest;
		OnBestActorChanged.Broadcast(NewBest, OldBest);
	}
}

void URangeComponent::NotifyIfBestMatchingActorsChanged()
{
	for (auto It = FilterRecords.CreateIterator(); It; ++It)
	{
		UActorFilter* Filter = It->Filter.Get();
		if (!Filter)
		{
			// The caller dropped its reference to a tracked filter without calling UntrackFilter first.
			It.RemoveCurrentSwap();
			continue;
		}

		AActor* OldBest = It->BestActor.Get();
		AActor* NewBest = FindBestMatchingActor(Filter);

		if (NewBest != OldBest)
		{
			It->BestActor = NewBest;
			OnBestMatchingActorChanged.Broadcast(Filter, NewBest, OldBest);
		}
	}
}

void URangeComponent::Capsule_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                             bool bFromSweep, const FHitResult& SweepResult)
{
	for (FInRangeInfo& InRangeInfo : InRangeInfos)
	{
		if (InRangeInfo.Actor == OtherActor)
		{
			++InRangeInfo.NbOccurrences;
			return;
		}
	}
	InRangeInfos.Emplace(OtherActor);
}

void URangeComponent::Capsule_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	for (auto It = InRangeInfos.CreateIterator(); It; ++It)
	{
		if (It->Actor == OtherActor)
		{
			if (--It->NbOccurrences == 0)
			{
				It.RemoveCurrentSwap();
			}
			break;
		}
	}
}

URangeComponent::FInRangeInfo::FInRangeInfo(AActor* Actor)
	: Actor(Actor), NbOccurrences(1), Score(TNumericLimits<float>::Lowest())
{
}

URangeComponent::FFilterRecord::FFilterRecord(UActorFilter* Filter, AActor* BestActor)
	: Filter(Filter), NbOccurrences(1), BestActor(BestActor)
{
}
