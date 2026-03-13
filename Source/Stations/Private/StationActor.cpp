#include "StationActor.h"
#include "HolderComponent.h"
#include "ItemActor.h"
#include "StationAsset.h"
#include "ActivityExecutor.h"
#include "RecipeSystem.h"
#include <Net/UnrealNetwork.h>

DEFINE_LOG_CATEGORY_STATIC(MS_StationActor, Verbose, All);

AStationActor::AStationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	ItemHolder = CreateDefaultSubobject<UHolderComponent>(TEXT("Item Holder"));
	ItemHolder->SetupAttachment(StaticMesh);

	Executor = CreateDefaultSubobject<UActivityExecutor>(TEXT("Activity Executor"));
}

void AStationActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
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
		return;
	
	URecipeSystem* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>();
	check(RecipeSystem);

	FGameplayTagContainer InteractionTags = StationAsset->ImplementedActivities;
	if (auto* ItemActor = Cast<AItemActor>(ItemHolder->GetCarriable()))
	{
		InteractionTags.AppendTags(ItemActor->GetItemTags());
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

void AStationActor::ApplyStationAsset()
{
	check(IsValid(StationAsset));

	StaticMesh->SetStaticMesh(StationAsset->StaticMesh);

	ItemHolder->AttachToComponent(StaticMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, StationAsset->HolderSocket);
}
