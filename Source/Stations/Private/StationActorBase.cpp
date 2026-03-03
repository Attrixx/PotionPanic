#include "StationActorBase.h"
#include "HolderComponent.h"
#include "Recipes/Public/RecipeSystem.h"

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
	UE_LOG(LogTemp, Log, TEXT("Station '%s' interacted by player"), *GetName());

	if (!StationAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Station '%s' has no StationAsset. Interaction ignored."), *GetName());
		return;
	}

	if (!ItemHolder)
	{
		UE_LOG(LogTemp, Warning, TEXT("Station '%s' has no ItemHolder. Interaction ignored."), *GetName());
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("Station '%s' has no valid World. Interaction ignored."), *GetName());
		return;
	}
	
	URecipeSystem* RecipeSystem = World->GetSubsystem<URecipeSystem>();
	if (!RecipeSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Station '%s' could not find RecipeSystem. Interaction ignored."), *GetName());
		return;
	}
	
	auto Response = RecipeSystem->GetRecipeStep(ItemHolder, StationAsset->Activities);
	
	if (Response.Interactions.IsEmpty() || CachedInstigator != nullptr)
	{
		return;
	}
	
	ResetCurrentInteractions();
	CachedInstigator = InInstigator;
	CachedInteractions = Response.Interactions;
	
	ExecuteNextInteraction();
}

void AStationActorBase::OnInteractionFinished(FInteractionOutput InteractionOutput)
{
	UE_LOG(LogTemp, Warning, TEXT("Interaction Complete"));
	
	if (InteractionOutput.InteractionResult == EInteractionResult::Success)
	{
		ExecuteNextInteraction();
	}
	else
	{
		ResetCurrentInteractions();
	}
}

void AStationActorBase::ResetCurrentInteractions()
{
	CachedInstigator = nullptr;
	CachedInteractions.Reset();
	InteractionIndex = -1;
}

void AStationActorBase::ExecuteNextInteraction()
{
	++InteractionIndex;
	
	if (InteractionIndex == CachedInteractions.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("Transformation success"));
		ResetCurrentInteractions();
		// TODO Instantiate Output item in socket and get rid of previous
	}
	else
	{
		FInteractionContext InteractionContext;
		InteractionContext.Instigator = CachedInstigator;
		InteractionContext.OnInteractionFinished.AddDynamic(this, &AStationActorBase::OnInteractionFinished);

		UInteractionBase* CurrentInteraction = CachedInteractions[InteractionIndex];
		if (!CurrentInteraction)
		{
			UE_LOG(LogTemp, Warning, TEXT("Station '%s' encountered a null interaction. Resetting sequence."), *GetName());
			ResetCurrentInteractions();
			return;
		}
		
		CurrentInteraction->StartInteraction(InteractionContext);
	}
}
