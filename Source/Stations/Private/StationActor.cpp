#include "StationActor.h"
#include "ActivityAsset.h"
#include "ActivityExecutor.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "ItemTags.h"
#include "RecipeSystem.h"
#include "StationAsset.h"
#include "StationVisualActor.h"
#include "NetworkSoundSubsystem.h"

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

	if (StationAsset)
	{
		ApplyStationAsset();
	}
}

void AStationActor::BeginPlay()
{
	Super::BeginPlay();

	Executor->Initialize(ItemHolder);
	ItemHolder->OnCarriableChanged.AddDynamic(this, &AStationActor::Holder_OnCarriableChanged);
	Executor->OnExecutionStatusChanged.AddDynamic(this, &AStationActor::Executor_OnExecutionStatusChanged);
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

UActivityExecutor* AStationActor::GetActivityExecutor() const
{
	return Executor;
}

void AStationActor::TryStartMatchingActivity(AActor* InInstigator)
{
	if (!HasAuthority())
	{
		// Reached on clients through Holder_OnCarriableChanged, which OnRep_Carriable broadcasts.
		// Activity execution and item ownership are both server side.
		return;
	}

	if (bStartingActivity)
	{
		// Guard re-entering from TryPickup broadcast
		return;
	}

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
		// TransferTo broadcasts OnCarriableChanged, which re-enters through Holder_OnCarriableChanged.
		TGuardValue<bool> ReentrancyGuard(bStartingActivity, true);

		if (!SourceHolder->TransferTo(ItemHolder))
		{
			// The item stayed in the instigator's hands: starting now would run the activity on an
			// empty station.
			UE_LOGFMT(MS_StationActor, Warning, "Could not take the instigator's item onto '{0}'.", GetName());
			return;
		}
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

	auto MatchWith = [&](const AItemActor* Item)
	{
		FGameplayTagContainer InteractionTags = StationAsset->ImplementedActivities;
		if (Item)
		{
			InteractionTags.AppendTags(Item->GetItemTags());
		}
		else
		{
			InteractionTags.AddTag(GameTags::Item_None);
		}

		return RecipeSystem->FindActivityByInputTags(InteractionTags);
	};

	// The station's own item comes first: it is already in place, and it occupies the station
	// whether or not it matches anything, so there is no falling back on an empty-handed activity.
	if (const AItemActor* OwnItem = Cast<AItemActor>(ItemHolder->GetCarriable()))
	{
		return MatchWith(OwnItem);
	}

	// Then whatever the instigator is holding, if the station and the activity both accept it.
	if (InInstigator && StationAsset->bCanEverTakeItemFromInstigator)
	{
		if (UHolderComponent* InstigatorHolder = InInstigator->FindComponentByClass<UHolderComponent>())
		{
			if (const AItemActor* InstigatorItem = Cast<AItemActor>(InstigatorHolder->GetCarriable()))
			{
				UActivityAsset* Activity = MatchWith(InstigatorItem);
				if (Activity && Activity->bCanTakeItemFromInstigator)
				{
					OutSourceHolder = InstigatorHolder;
					return Activity;
				}
			}
		}
	}

	// Nothing usable in hand: fall back on what the station does empty-handed. Reaching this with
	// full hands is nominal, the item just has no use here.
	return MatchWith(nullptr);
}

void AStationActor::Holder_OnCarriableChanged(UHolderComponent* Holder)
{
	check(Holder == ItemHolder);

	// Only fetch if an item was put on the holder
	if (!Cast<AItemActor>(ItemHolder->GetCarriable()))
	{
		return;
	}

	TryStartMatchingActivity(nullptr);

	// Re-entered from an item being moved on or off this holder: whoever is moving it is still
	// deciding what happens to it.
	if (!HasAuthority() || bStartingActivity)
	{
		return;
	}

	// The item was caught, nothing had a use for it, and this station is no storage spot.
	const bool bBusy = Executor->GetExecutionStatus() == EActivityExecutionStatus::Ongoing;
	if (StationAsset && !StationAsset->bCanStoreItems && !bBusy && ItemHolder->GetCarriable())
	{
		ItemHolder->Eject();
	}
}

void AStationActor::Executor_OnExecutionStatusChanged(UActivityExecutor* InExecutor, EActivityExecutionStatus NewStatus)
{
	UNetworkSoundSubsystem* SoundSubsystem = GetGameInstance()->GetSubsystem<UNetworkSoundSubsystem>();
	check(SoundSubsystem);

	switch (NewStatus)
	{
	case EActivityExecutionStatus::Ongoing:
		if (StationAsset)
		{
			if (StationAsset->OnInteractSound)
			{
				SoundSubsystem->PlayNetworkedSound(StationAsset->OnInteractSound, GetActorLocation(), this);
			}
			if (StationAsset->OnActivityGoingSound)
			{
				OnActivityGoindSoundHandle = SoundSubsystem->PlayNetworkedSound(StationAsset->OnActivityGoingSound, GetActorLocation(), this);
			}
		}
		break;
	case EActivityExecutionStatus::Success:
		if (StationAsset)
		{
			if (StationAsset->OnActivitySuccessSound)
			{
				SoundSubsystem->PlayNetworkedSound(StationAsset->OnActivitySuccessSound, GetActorLocation(), this);
			}
			if (StationAsset->OnActivityGoingSound && OnActivityGoindSoundHandle != -1)
			{
				SoundSubsystem->StopNetworkedSound(OnActivityGoindSoundHandle);
				OnActivityGoindSoundHandle = -1;
			}
		}
		break;
	case EActivityExecutionStatus::Failed:
	case EActivityExecutionStatus::Canceled:
		if (StationAsset)
		{
			if (StationAsset->OnActivityFailedSound)
			{
				SoundSubsystem->PlayNetworkedSound(StationAsset->OnActivityFailedSound, GetActorLocation(), this);
			}
			if (StationAsset->OnActivityGoingSound && OnActivityGoindSoundHandle != -1)
			{
				SoundSubsystem->StopNetworkedSound(OnActivityGoindSoundHandle);
				OnActivityGoindSoundHandle = -1;
			}
		}
		break;
	}
	
	if (NewStatus == EActivityExecutionStatus::Ongoing || !HasAuthority())
	{
		return;
	}

	if (!StationAsset || StationAsset->bCanStoreItems)
	{
		return;
	}

	if (!ItemHolder->GetCarriable())
	{
		// Nominal case for a pass-through station: the conclusion consumed the item.
		return;
	}

	// The activity left the item behind, but this station is not a storage spot.
	AActor* LastInstigator = InExecutor->GetExecutionState().LastInstigator.Get();
	UHolderComponent* InstigatorHolder = LastInstigator ? LastInstigator->FindComponentByClass<UHolderComponent>() : nullptr;

	// Moving the item off this station re-enters through Holder_OnCarriableChanged: the leftover is
	// on its way out, it must not start a new activity here on the way.
	TGuardValue<bool> ReentrancyGuard(bStartingActivity, true);

	if (!InstigatorHolder || !ItemHolder->TransferTo(InstigatorHolder))
	{
		ItemHolder->Eject();
	}
}

void AStationActor::OnRep_StationAsset()
{
	ApplyStationAsset();
}

void AStationActor::DropItemRefusedByAsset()
{
	const UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		ItemHolder->Eject();
		return;
	}

	// No physics to eject into outside a game world, and this runs from OnConstruction: unhook the
	// item where it stands and leave it to the level author.
	UE_LOGFMT(MS_StationActor, Error,
		"'{0}' holds an item its station asset refuses to store. Detached in place.", GetName());
	ItemHolder->Detach();
}

// This method MUST be callable in Editor, don't call gameplay stuff in here!!
void AStationActor::ApplyStationAsset()
{
	// Catching is its own axis: a bin or a delivery counter is no storage spot, yet it has to be
	// able to take what is thrown at it.
	ItemHolder->SetCatchAllowed(!StationAsset || StationAsset->bCanCatchItems);

	const bool bCanStoreItems = !StationAsset || StationAsset->bCanStoreItems;

	// Swapping the asset can leave an item on a station that is no longer allowed to hold one.
	if (!bCanStoreItems && ItemHolder->GetCarriable() && HasAuthority())
	{
		DropItemRefusedByAsset();
	}

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
