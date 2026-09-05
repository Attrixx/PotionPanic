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

#include <Components/BoxComponent.h>
#include <Components/ChildActorComponent.h>
#include <Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY_STATIC(MS_StationActor, Verbose, All);

AStationActor::AStationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Every station is a box; only its size varies, and that comes from the station asset.
	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Body->SetGenerateOverlapEvents(true);

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

	// After the bindings: the starting item must go through Holder_OnCarriableChanged like any
	// other, so a station that cannot store it reacts to it now rather than keeping it forever.
	SpawnStartingItem();
}

void AStationActor::SpawnStartingItem()
{
	// Items are replicated: the server spawns them, clients receive them.
	if (!HasAuthority() || !StartingItem)
	{
		return;
	}

	if (!StartingItemClass)
	{
		UE_LOGFMT(MS_StationActor, Error, "'{0}' has a starting item but no actor class to spawn it in.", GetName());
		return;
	}

	AItemActor* NewItem = GetWorld()->SpawnActor<AItemActor>(StartingItemClass);
	if (!NewItem)
	{
		UE_LOGFMT(MS_StationActor, Error, "'{0}' failed to spawn its starting item.", GetName());
		return;
	}

	NewItem->SetItemAsset(StartingItem);

	// Spawned at the world origin until the holder takes it: a refusal here would leave it lying
	// there, so it goes rather than littering the level.
	if (!ItemHolder->TryPickup(NewItem))
	{
		UE_LOGFMT(MS_StationActor, Error, "'{0}' could not take its starting item onto its holder.", GetName());
		NewItem->Destroy();
	}
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

	bReturnItemToInstigator = SourceHolder != nullptr
		&& Activity->TakeFromInstigator == EActivityTakeFromInstigator::TakeAndReturn;

	Executor->StartActivity(Activity, InInstigator, SourceHolder != nullptr);
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

	// Empty hands are a state an activity can require, not the absence of one: they match as
	// Item.None rather than as nothing at all.
	auto TagsOf = [](const AItemActor* Item)
	{
		return Item ? Item->GetItemTags() : FGameplayTagContainer(GameTags::Item_None);
	};

	UHolderComponent* InstigatorHolder = InInstigator ? InInstigator->FindComponentByClass<UHolderComponent>() : nullptr;
	const AItemActor* InstigatorItem = InstigatorHolder ? Cast<AItemActor>(InstigatorHolder->GetCarriable()) : nullptr;

	// No instigator at all is not the same state as an instigator with empty hands: an activity
	// requiring Item.None from the player requires a player. An empty container satisfies an activity
	// that leaves InstigatorItemTags empty, and nothing else.
	const FGameplayTagContainer InstigatorTags = InInstigator ? TagsOf(InstigatorItem) : FGameplayTagContainer();

	// The station's own item comes first: it is already in place, and it occupies the station
	// whether or not it matches anything, so there is nothing to take from the instigator.
	if (const AItemActor* OwnItem = Cast<AItemActor>(ItemHolder->GetCarriable()))
	{
		return RecipeSystem->FindActivity(TagsOf(OwnItem), StationAsset->ImplementedActivities, InstigatorTags);
	}

	// The station's holder is free. Taking the instigator's item onto it comes next: the item is
	// matched as the station's own, and the instigator ends up empty-handed.
	if (InstigatorItem && StationAsset->bCanEverTakeItemFromInstigator)
	{
		UActivityAsset* Activity = RecipeSystem->FindActivity(
			TagsOf(InstigatorItem),
			StationAsset->ImplementedActivities,
			FGameplayTagContainer(GameTags::Item_None));

		if (Activity && Activity->TakeFromInstigator != EActivityTakeFromInstigator::Never)
		{
			OutSourceHolder = InstigatorHolder;
			return Activity;
		}
	}

	// Nothing was taken: fall back on what this station does empty, with the instigator keeping
	// whatever it holds. Reaching this with full hands is nominal, the item just has no use here.
	return RecipeSystem->FindActivity(
		FGameplayTagContainer(GameTags::Item_None),
		StationAsset->ImplementedActivities,
		InstigatorTags);
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
				SoundSubsystem->PlayLocalSound(StationAsset->OnInteractSound, GetActorLocation());
			}
			if (StationAsset->OnActivityGoingSound)
			{
				OnActivityGoindSoundHandle = SoundSubsystem->PlayLocalSound(StationAsset->OnActivityGoingSound, GetActorLocation());
			}
		}
		break;
	case EActivityExecutionStatus::Success:
		if (StationAsset)
		{
			if (StationAsset->OnActivitySuccessSound)
			{
				SoundSubsystem->PlayLocalSound(StationAsset->OnActivitySuccessSound, GetActorLocation());
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
				SoundSubsystem->PlayLocalSound(StationAsset->OnActivityFailedSound, GetActorLocation());
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

	const bool bWasBorrowed = std::exchange(bReturnItemToInstigator, false);
	const bool bIsStorage = StationAsset && StationAsset->bCanStoreItems;
	if (!bWasBorrowed && bIsStorage)
	{
		return;
	}

	if (!ItemHolder->GetCarriable())
	{
		// Nothing left to hand back or push out: the item was consumed.
		return;
	}

	// Moving the item off this station re-enters through Holder_OnCarriableChanged: the leftover is
	// on its way out, it must not start a new activity here on the way.
	TGuardValue<bool> ReentrancyGuard(bStartingActivity, true);

	const FActivityExecutionState& State = InExecutor->GetExecutionState();
	UHolderComponent* InstigatorHolder = State.InstigatorHolder.Get();

	if (InstigatorHolder && ItemHolder->TransferTo(InstigatorHolder))
	{
		return;
	}

	// A borrowed item the instigator cannot take back stays put, unless this station is no storage
	// spot -- then it has to go somewhere, and the floor is the only option left.
	if (!bIsStorage)
	{
		ItemHolder->Eject();
	}
	else
	{
		UE_LOGFMT(MS_StationActor, Warning,
			"'{0}' could not hand its borrowed item back: the instigator is gone or full.", GetName());
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
		ItemHolder->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		VisualActor->SetChildActorClass(VisualClass);
	}

	// A station with no asset keeps whatever the class defaults hold, rather than collapsing to a
	// zero-sized box nothing can touch.
	if (StationAsset)
	{
		const FVector BodyExtent = StationAsset->BodyExtent;
		Body->SetBoxExtent(BodyExtent);
		Body->SetRelativeLocation(FVector(0.f, 0.f, BodyExtent.Z));
	}

	if (AStationVisualActor* Visual = Cast<AStationVisualActor>(VisualActor->GetChildActor()))
	{
		Visual->SetStationActor(this);

		FName SocketName = NAME_None;
		USceneComponent* Anchor = Visual->GetItemAnchor(SocketName);

		if (ItemHolder->GetAttachParent() != Anchor || ItemHolder->GetAttachSocketName() != SocketName)
		{
			// IncludingScale, not NotIncludingScale: the latter is KeepWorld on scale, which preserves
			// whatever scale the holder currently has. Construction script instance data restores
			// that value, so a visual that was once scaled leaves the holder permanently resized --
			// and the holder's scale is its overlap radius. SnapToTarget resets it to 1 every time.
			ItemHolder->AttachToComponent(Anchor, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
		}
	}
}
