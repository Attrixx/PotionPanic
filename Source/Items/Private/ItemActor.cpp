// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemActor.h"
#include "ItemAsset.h"
#include "ItemTags.h"
#include "ItemVisualActor.h"
#include <Net/UnrealNetwork.h>
#include <Components/CapsuleComponent.h>
#include <Components/ChildActorComponent.h>

DEFINE_LOG_CATEGORY_STATIC(MS_ItemActor, Log, All);

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);

	Body = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetNotifyRigidBodyCollision(true);
	Body->BodyInstance.bLockXRotation = true;
	Body->BodyInstance.bLockYRotation = true;

	VisualActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("Visual Actor"));
	VisualActor->SetupAttachment(Body);
}

void AItemActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AItemActor, ItemAsset)
	DOREPLIFETIME(AItemActor, ItemTags)
}

void AItemActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (ItemAsset)
	{
		ApplyItemAsset();
	}
}

void AItemActor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
                           FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Only the authority simulates the item, so only it ever gets here with a real impact.
	if (!bBroken && HasAuthority() && CanBreak())
	{
		bBroken = true;

		// The visual is called first so it can spawn its break effects, then the item goes. The
		// multicast is reliable: it is the last thing anyone hears about this item, and losing it
		// would leave the item vanishing with no explanation.
		Multicast_Broken();
		Destroy();
		return;
	}

	constexpr float GroundCollisionThreshold = 0.8f;
	if (Hit.ImpactNormal.Dot(FVector::UpVector) >= GroundCollisionThreshold)
	{
		Body->SetSimulatePhysics(false);
	}
}

void AItemActor::SetItemAsset(UItemAsset* NewItemAsset)
{
	if (HasAuthority())
	{
		ItemAsset = NewItemAsset;
		OnRep_ItemAsset();
	}
}

void AItemActor::SetItemTags(const FGameplayTagContainer& NewItemTags)
{
	check(NewItemTags == NewItemTags.Filter(FGameplayTagContainer(GameTags::Item)));
	ItemTags = NewItemTags;
	NotifyItemTagsChanged();
}

void AItemActor::AppendItemTags(const FGameplayTagContainer& NewItemTags)
{
	check(NewItemTags == NewItemTags.Filter(FGameplayTagContainer(GameTags::Item)));
	ItemTags.AppendTags(NewItemTags);
	NotifyItemTagsChanged();
}

void AItemActor::RemoveItemTag(const FGameplayTagContainer& ItemTagsToRemove)
{
	check(ItemTagsToRemove == ItemTagsToRemove.Filter(FGameplayTagContainer(GameTags::Item)));
	ItemTags.RemoveTags(ItemTagsToRemove);
	NotifyItemTagsChanged();
}

bool AItemActor::CanBreak() const
{
	// Read from the tags, not the asset: an activity can make an item breakable along the way.
	return ItemTags.HasTag(GameTags::Item_Breakable);
}

UPrimitiveComponent* AItemActor::GetPrimitive_Implementation() const
{
	return Body;
}

FName AItemActor::GetStandaloneCollisionProfileName_Implementation() const
{
	return StandaloneCollisionProfileName;
}

FName AItemActor::GetCarriedCollisionProfileName_Implementation() const
{
	return CarriedCollisionProfileName;
}

bool AItemActor::CanBeThrown_Implementation() const
{
	// An item without an asset is not a gameplay item yet: nothing forbids throwing it.
	return !ItemAsset || ItemAsset->bCanBeThrown;
}

void AItemActor::OnThrow_Implementation(FVector Velocity)
{
	if (HasAuthority())
	{
		// Runs here too: a multicast sent from the authority executes locally as well.
		Multicast_Thrown(Velocity);
		return;
	}

	// Called locally on a client, by Blueprint: play it where we are, nothing to send.
	if (AItemVisualActor* Visual = GetVisual())
	{
		Visual->OnItemThrown(Velocity);
	}
}

void AItemActor::Multicast_Thrown_Implementation(FVector Velocity)
{
	if (AItemVisualActor* Visual = GetVisual())
	{
		Visual->OnItemThrown(Velocity);
	}
}

void AItemActor::Multicast_Broken_Implementation()
{
	if (AItemVisualActor* Visual = GetVisual())
	{
		Visual->OnItemBroken();
	}
}

void AItemActor::OnRep_ItemTags()
{
	NotifyItemTagsChanged();
}

void AItemActor::OnRep_ItemAsset()
{
	if (IsValid(ItemAsset))
	{
		ApplyItemAsset();
	}
}

void AItemActor::ApplyItemAsset()
{
	check(IsValid(ItemAsset));

	ItemTags = ItemAsset->ItemTags;

	TSubclassOf<AItemVisualActor> VisualClass = ItemAsset->VisualActorClass;
	if (VisualActor->GetChildActorClass() != VisualClass)
	{
		VisualActor->SetChildActorClass(VisualClass);
	}

	// Before the tag notification below: a visual must know its item by the time it hears anything.
	if (AItemVisualActor* Visual = GetVisual())
	{
		Visual->SetItemActor(this);
	}

	NotifyItemTagsChanged();

	// A capsule whose half height is under its radius is a broken shape: clamp rather than let the
	// physics engine sort it out.
	Body->SetCapsuleSize(ItemAsset->BodyRadius, FMath::Max(ItemAsset->BodyHalfHeight, ItemAsset->BodyRadius));
}

void AItemActor::NotifyItemTagsChanged()
{
	// A client reaches this twice for one change: once from OnRep_ItemAsset, which re-derives the
	// tags locally, and once from OnRep_ItemTags. Only an actual change is passed on.
	if (ItemTags == LastNotifiedTags)
	{
		return;
	}

	AItemVisualActor* Visual = GetVisual();
	if (!Visual)
	{
		// No visual yet: leave LastNotifiedTags alone so the next call still delivers these tags.
		return;
	}

	LastNotifiedTags = ItemTags;
	Visual->OnItemTagsChanged(ItemTags);
}

AItemVisualActor* AItemActor::GetVisual() const
{
	return Cast<AItemVisualActor>(VisualActor->GetChildActor());
}
