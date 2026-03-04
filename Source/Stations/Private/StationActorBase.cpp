#include "StationActorBase.h"
#include "HolderComponent.h"
#include "Recipes/Public/RecipeSystem.h"

DEFINE_LOG_CATEGORY_STATIC(MS_StationActorBase, Log, All);

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

void AStationActorBase::Interact(APlayerController* InInstigator)
{
	// Station interaction is handled by external manager
	// This method satisfies the IInteractable interface
	UE_LOG(MS_StationActorBase, Log, TEXT("Station '%s' interacted by player"), *GetName());

	if (!StationAsset)
	{
		UE_LOG(MS_StationActorBase, Warning, TEXT("Station '%s' has no StationAsset. Interaction ignored."), *GetName());
		return;
	}

	if (!ItemHolder)
	{
		UE_LOG(MS_StationActorBase, Warning, TEXT("Station '%s' has no ItemHolder. Interaction ignored."), *GetName());
		return;
	}
	
	if (!bHasInteractions)
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
		bHasInteractions = true;
	}
	
	if (bIsBusy)
	{
		CachedInteractionInfos[InteractionIndex].Interaction->InteractWhileProcess();
	}
	else
	{
		ExecuteNextInteraction(InInstigator);	
	}
}

void AStationActorBase::OnInteractionFinished(FInteractionOutput InteractionOutput)
{
	UE_LOG(MS_StationActorBase, Warning, TEXT("Interaction Complete"));
	
	bIsBusy = false;
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
	bHasInteractions = false;
	bIsBusy = false;
}

void AStationActorBase::ExecuteNextInteraction(APlayerController* InInstigator)
{
	++InteractionIndex;
	
	if (InteractionIndex == CachedInteractionInfos.Num())
	{
		UE_LOG(MS_StationActorBase, Warning, TEXT("Transformation success"));
		ResetCurrentInteractions();
		// TODO Instantiate Output item in socket and get rid of previous
	}
	else if (InInstigator || !CachedInteractionInfos[InteractionIndex].bRequiresPlayerInteraction)
	{
		FInteractionContext InteractionContext;
		InteractionContext.Instigator = InInstigator;
		InteractionContext.OnInteractionFinished.AddDynamic(this, &AStationActorBase::OnInteractionFinished);

		UInteractionBase* CurrentInteraction = CachedInteractionInfos[InteractionIndex].Interaction;
		if (!CurrentInteraction)
		{
			UE_LOG(MS_StationActorBase, Warning, TEXT("Station '%s' encountered a null interaction. Resetting sequence."), *GetName());
			ResetCurrentInteractions();
			return;
		}
		
		bIsBusy = true;
		CurrentInteraction->StartInteraction(InteractionContext);
	}
	else
	{
		--InteractionIndex;
	}
}
