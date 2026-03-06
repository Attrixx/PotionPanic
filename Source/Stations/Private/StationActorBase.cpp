#include "StationActorBase.h"
#include "HolderComponent.h"
#include "StationAsset.h"
#include "Interactions/Public/InteractionBase.h"
#include "Recipes/Public/RecipeSystem.h"

DEFINE_LOG_CATEGORY_STATIC(MS_StationActorBase, Verbose, All);

AStationActorBase::AStationActorBase()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);

	ItemHolder = CreateDefaultSubobject<UHolderComponent>(TEXT("Item Holder"));
	ItemHolder->SetupAttachment(StaticMesh);
}

void AStationActorBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (StationAsset)
	{
		SetStationAsset(StationAsset);
	}
}

void AStationActorBase::BeginPlay()
{
	Super::BeginPlay();
}

void AStationActorBase::SetStationAsset(UStationAsset* NewAsset)
{
	if (!NewAsset)
		return;

	StationAsset = NewAsset;

	StaticMesh->SetStaticMesh(NewAsset->StaticMesh);
}

void AStationActorBase::Interact(AActor* InInstigator)
{
	// Station interaction is handled by external manager
	// This method satisfies the IInteractable interface
	UE_LOG(MS_StationActorBase, Verbose, TEXT("Station '%s' interacted by player"), *GetName());

	if (!StationAsset)
	{
		UE_LOG(MS_StationActorBase, Warning, TEXT("Station '%s' has no StationAsset. Interaction ignored."),
		       *GetName());
		return;
	}

	if (!ItemHolder)
	{
		UE_LOG(MS_StationActorBase, Warning, TEXT("Station '%s' has no ItemHolder. Interaction ignored."), *GetName());
		return;
	}

	switch (Status)
	{
		case EStationStatus::Idle:
		{
			URecipeSystem* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>();
			check(RecipeSystem);

			auto Response = RecipeSystem->GetRecipeStep(ItemHolder, StationAsset->Activities);
			if (Response.InteractionInfos.IsEmpty())
			{
				return;
			}

			ResetCurrentInteractions();
			CachedInteractionInfos = Response.InteractionInfos;
			Status = EStationStatus::Ready;
			[[fallthrough]];
		}
		case EStationStatus::Ready:
		{
			ExecuteNextInteraction(InInstigator);
			break;
		}
		case EStationStatus::Busy:
		{
			CachedInteractionInfos[InteractionIndex].Interaction->InteractWhileProcess();
			break;
		}
	}
}

void AStationActorBase::OnInteractionFinished(const FInteractionOutput& InteractionOutput)
{
	UE_LOG(MS_StationActorBase, Verbose, TEXT("Interaction Complete"));

	Status = EStationStatus::Ready;
	if (InteractionOutput.InteractionResult == EInteractionResult::Success)
	{
		ExecuteNextInteraction(nullptr);
	}
	else
	{
		ResetCurrentInteractions();
	}
}

void AStationActorBase::ResetCurrentInteractions()
{
	CachedInteractionInfos.Reset();
	InteractionIndex = -1;
	Status = EStationStatus::Idle;
}

void AStationActorBase::ExecuteNextInteraction(AActor* InInstigator)
{
	++InteractionIndex;

	if (InteractionIndex == CachedInteractionInfos.Num())
	{
		UE_LOG(MS_StationActorBase, Error, TEXT("Not implemented"));
		ResetCurrentInteractions();
		// TODO Change item asset or delete it
	}
	else if (InInstigator || !CachedInteractionInfos[InteractionIndex].bRequiresPlayerInteraction)
	{
		FInteractionContext InteractionContext;
		InteractionContext.Instigator = InInstigator;
		InteractionContext.OnInteractionFinished.AddDynamic(this, &AStationActorBase::OnInteractionFinished);

		UInteractionBase* CurrentInteraction = CachedInteractionInfos[InteractionIndex].Interaction;
		if (!CurrentInteraction)
		{
			UE_LOG(MS_StationActorBase, Warning,
			       TEXT("Station '%s' encountered a null interaction. Resetting sequence."), *GetName());
			ResetCurrentInteractions();
			return;
		}

		Status = EStationStatus::Busy;
		CurrentInteraction->StartInteraction(InteractionContext);
	}
	else
	{
		--InteractionIndex;
	}
}
