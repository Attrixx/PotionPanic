#include "StationActor.h"
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
	FetchInstructions(InInstigator);
	Executor->Interact(InInstigator);
}

void AStationActor::SetStationAsset(UStationAsset* NewStationAsset)
{
	if (HasAuthority())
	{
		StationAsset = NewStationAsset;
		OnRep_StationAsset();
	}
}

void AStationActor::FetchInstructions(AActor* InInstigator)
{
	if (Executor->GetExecutionStatus() == EActivityExecutionStatus::Ongoing)
	{
		// Calling StartActivity again would cancel the current activity
		return;
	}

	URecipeSystem* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>();
	check(RecipeSystem);

	FGameplayTagContainer InteractionTags = StationAsset->ImplementedActivities;
	if (auto* ItemActor = Cast<AItemActor>(ItemHolder->GetCarriable()))
	{
		InteractionTags.AppendTags(ItemActor->GetItemTags());
	}
	else
	{
		InteractionTags.AddTag(GameTags::Item_None);
	}

	if (UActivityAsset* Activity = RecipeSystem->FindActivityByInputTags(InteractionTags))
	{
		Executor->StartActivity(Activity, InInstigator);
	}
}

void AStationActor::Holder_OnCarriableChanged(UHolderComponent* Holder)
{
	check(Holder == ItemHolder);

	// Only fetch if an item was put on the holder
	if (Cast<AItemActor>(ItemHolder->GetCarriable()))
	{
		FetchInstructions(nullptr);
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
		VisualActor->SetChildActorClass(VisualClass);
	}

	if (AStationVisualActor* Visual = Cast<AStationVisualActor>(VisualActor->GetChildActor()))
	{
		FName SocketName = NAME_None;
		if (USceneComponent* Anchor = Visual->GetItemAnchor(SocketName))
		{
			ItemHolder->AttachToComponent(Anchor, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
			return;
		}
	}

	// No VisualActor or no Anchor
	ItemHolder->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}
