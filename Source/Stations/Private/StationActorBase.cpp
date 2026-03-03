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
	
	URecipeSystem* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>();
	check(RecipeSystem);
	
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
		
		CachedInteractions[InteractionIndex]->StartInteraction(InteractionContext);
	}
}
