#include "StationActor.h"
#include "ActivityAsset.h"
#include "ActivityExecutor.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ItemTags.h"
#include "RecipeSystem.h"
#include "StationAsset.h"
#include "StationVisualActor.h"
#include <Components/ChildActorComponent.h>
#include <Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY_STATIC(MS_StationActor, Verbose, All);

AStationActor::AStationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	ItemHolder = CreateDefaultSubobject<UHolderComponent>(TEXT("Item Holder"));
	ItemHolder->SetupAttachment(RootComponent);

	Executor = CreateDefaultSubobject<UActivityExecutor>(TEXT("Activity Executor"));

	VisualActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("Visual Actor"));
	VisualActor->SetupAttachment(RootComponent);
}

void AStationActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStationActor, StationAsset);
}

void AStationActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyStationAsset();
}

void AStationActor::BeginPlay()
{
	Super::BeginPlay();

	Executor->Initialize(ItemHolder);
	ItemHolder->OnCarriableChanged.AddDynamic(this, &AStationActor::Holder_OnCarriableChanged);
}

void AStationActor::Interact_Implementation(AActor* InInstigator)
{
	TryStartMatchingActivity(InInstigator);
	Executor->Interact(InInstigator);
}

bool AStationActor::CanInteract_Implementation(AActor* InInstigator) const
{
	UHolderComponent* UnusedSourceHolder = nullptr;
	return Executor->GetExecutionStatus() == EActivityExecutionStatus::Ongoing
		|| FindMatchingActivity(InInstigator, UnusedSourceHolder) != nullptr;
}

void AStationActor::SetStationAsset(UStationAsset* NewStationAsset)
{
	if (HasAuthority())
	{
		StationAsset = NewStationAsset;
		OnRep_StationAsset();
	}
}

void AStationActor::TryStartMatchingActivity(AActor* InInstigator)
{
	if (Executor->GetExecutionStatus() == EActivityExecutionStatus::Ongoing)
	{
		// Calling StartActivity again would cancel the current activity
		return;
	}

	UHolderComponent* SourceHolder = nullptr;
	UActivityAsset* Activity = FindMatchingActivity(InInstigator, SourceHolder);
	if (!Activity)
	{
		return;
	}

	// Activities operate on this station's own ItemHolder. If the item that determined the
	// match is the instigator's rather than the station's, move it here before starting.
	if (SourceHolder)
	{
		auto* Item = SourceHolder->Release();
		ItemHolder->TryPickup(Item);
	}

	Executor->StartActivity(Activity, InInstigator);
}

UActivityAsset* AStationActor::FindMatchingActivity(AActor* InInstigator, UHolderComponent*& OutSourceHolder) const
{
	OutSourceHolder = nullptr;

	if (!StationAsset)
	{
		return nullptr;
	}

	URecipeSystem* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>();
	check(RecipeSystem);

	AItemActor* ItemActor = Cast<AItemActor>(ItemHolder->GetCarriable());
	if (!ItemActor && InInstigator && StationAsset->bCanEverTakeItemFromInstigator)
	{
		if (UHolderComponent* InstigatorHolder = InInstigator->FindComponentByClass<UHolderComponent>())
		{
			if (AItemActor* InstigatorItem = Cast<AItemActor>(InstigatorHolder->GetCarriable()))
			{
				ItemActor = InstigatorItem;
				OutSourceHolder = InstigatorHolder;
			}
		}
	}

	FGameplayTagContainer InteractionTags = StationAsset->ImplementedActivities;
	if (ItemActor)
	{
		InteractionTags.AppendTags(ItemActor->GetItemTags());
	}
	else
	{
		InteractionTags.AddTag(GameTags::Item_None);
	}

	UActivityAsset* FoundActivity = RecipeSystem->FindActivityByInputTags(InteractionTags);
	if (FoundActivity && !FoundActivity->bCanTakeItemFromInstigator && OutSourceHolder)
	{
		// Cancel, we can't use this activity by taking the item from instigator
		return nullptr;
	}
	return FoundActivity;
}

void AStationActor::Holder_OnCarriableChanged(UHolderComponent* Holder)
{
	check(Holder == ItemHolder);

	// Only fetch if an item was put on the holder
	if (Cast<AItemActor>(ItemHolder->GetCarriable()))
	{
		TryStartMatchingActivity(nullptr);
	}
}

void AStationActor::OnRep_StationAsset()
{
	ApplyStationAsset();
}

// This method MUST be callable in Editor, don't call gameplay stuff in here!!
void AStationActor::ApplyStationAsset()
{
	TSubclassOf<AStationVisualActor> VisualClass = StationAsset ? StationAsset->VisualActorClass : nullptr;

	if (VisualActor->GetChildActorClass() != VisualClass)
	{
		ItemHolder->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		VisualActor->SetChildActorClass(VisualClass);
	}

	if (AStationVisualActor* Visual = Cast<AStationVisualActor>(VisualActor->GetChildActor()))
	{
		FName SocketName = NAME_None;
		USceneComponent* Anchor = Visual->GetItemAnchor(SocketName);

		if (ItemHolder->GetAttachParent() != Anchor || ItemHolder->GetAttachSocketName() != SocketName)
		{
			ItemHolder->AttachToComponent(Anchor, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
		}
	}
}
