// Fill out your copyright notice in the Description page of Project Settings.

#include "RangeComponent.h"

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

void URangeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SortInRangeInfos();
	NotifyIfBestActorChanged();
	NotifyIfBestActorForInterfacesChanged();

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
		for (const FInterfaceRecord& Record : InterfaceRecords)
		{
			ShowDebug(Record.Interface->GetName(), Record.BestActor.Get());
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

AActor* URangeComponent::GetBestActorImplementing(TSubclassOf<UInterface> Interface) const
{
	for (const FInterfaceRecord& Record : InterfaceRecords)
	{
		if (Record.Interface == Interface)
			return Record.BestActor.Get();
	}

	UE_LOGFMT(MS_RangeComponent, Error, "GetBestActorImplementing: {0} is not tracked. Call TrackInterface first.", GetNameSafe(Interface));
	return nullptr;
}

AActor* URangeComponent::FindBestActorImplementing(TSubclassOf<UInterface> Interface) const
{
	// There should never be more than a few actors in range if the capsule is 
	// set up correctly, iterating is the best option.

	for (const FInRangeInfo& Info : InRangeInfos)
	{
		if (AActor* InfoActor = Info.Actor.Get())
		{
			UClass* InfoActorClass = InfoActor->GetClass();
			if (InfoActorClass && InfoActorClass->ImplementsInterface(Interface))
			{
				return InfoActor;
			}
		}
	}
	return nullptr;
}

void URangeComponent::TrackInterface(TSubclassOf<UInterface> Interface)
{
	FInterfaceRecord* FoundRecord = InterfaceRecords.FindByPredicate([Interface](const FInterfaceRecord& Record)
	{
		return Record.Interface == Interface;
	});

	if (!FoundRecord)
		FoundRecord = &InterfaceRecords.Emplace_GetRef(Interface, FindBestActorImplementing(Interface));

	++FoundRecord->NbOccurrences;
}

void URangeComponent::UntrackInterface(TSubclassOf<UInterface> Interface)
{
	int32 RecordIndex = InterfaceRecords.IndexOfByPredicate([Interface](const FInterfaceRecord& Record)
	{
		return Record.Interface == Interface;
	});

	if (RecordIndex == INDEX_NONE)
		return;

	auto& FoundRecord = InterfaceRecords[RecordIndex];
	if (--FoundRecord.NbOccurrences == 0)
	{
		InterfaceRecords.RemoveAt(RecordIndex);
	}
}

void URangeComponent::SortInRangeInfos()
{
	if (InRangeInfos.IsEmpty())
		return;

	const float MyCapsuleRadius = GetScaledCapsuleRadius();
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

void URangeComponent::NotifyIfBestActorForInterfacesChanged()
{
	for (FInterfaceRecord& Record : InterfaceRecords)
	{
		AActor* OldBest = Record.BestActor.Get();
		AActor* NewBest = FindBestActorImplementing(Record.Interface);

		if (NewBest != OldBest)
		{
			Record.BestActor = NewBest;
			OnBestActorImplementingChanged.Broadcast(Record.Interface, NewBest, OldBest);
		}
	}
}

void URangeComponent::Capsule_OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                             bool bFromSweep, const FHitResult& SweepResult)
{
	FInRangeInfo* InRangeInfo = InRangeInfos.FindByPredicate([OtherActor](const FInRangeInfo& Info)
	{
		return Info.Actor == OtherActor;
	});

	if (!InRangeInfo)
		InRangeInfo = &InRangeInfos.Emplace_GetRef(OtherActor);

	++InRangeInfo->NbOccurrences;
}

void URangeComponent::Capsule_OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	int32 InfoIndex = InRangeInfos.IndexOfByPredicate([OtherActor](const FInRangeInfo& Info)
	{
		return Info.Actor == OtherActor;
	});

	if (InfoIndex == INDEX_NONE)
		return;

	auto& InRangeInfo = InRangeInfos[InfoIndex];
	if (--InRangeInfo.NbOccurrences == 0)
	{
		InRangeInfos.RemoveAt(InfoIndex);
	}
}

URangeComponent::FInRangeInfo::FInRangeInfo(AActor* Actor)
	: Actor(Actor), NbOccurrences(0), Score(TNumericLimits<float>::Lowest())
{
}

URangeComponent::FInterfaceRecord::FInterfaceRecord(TSubclassOf<UInterface> Interface, AActor* BestActor)
	: Interface(Interface), NbOccurrences(0), BestActor(BestActor)
{
}
