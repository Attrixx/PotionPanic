#include "StationActorBase.h"
#include "HolderComponent.h"
#include "StationAsset.h"
#include "Activities/Public/ActivityStep.h"
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
		UE_LOG(MS_StationActorBase, Warning, TEXT("Station '%s' has no StationAsset. Activity ignored."),
		       *GetName());
		return;
	}

	if (!ItemHolder)
	{
		UE_LOG(MS_StationActorBase, Warning, TEXT("Station '%s' has no ItemHolder. Activity ignored."), *GetName());
		return;
	}

	switch (Status)
	{
		case EStationStatus::Idle:
		{
			URecipeSystem* RecipeSystem = GetWorld()->GetSubsystem<URecipeSystem>();
			check(RecipeSystem);

			auto Response = RecipeSystem->GetRecipeStep(ItemHolder, StationAsset->Activities);
			if (Response.ActivitySteps.IsEmpty())
			{
				return;
			}

			ResetCurrentActivities();
			CachedActivitySteps = Response.ActivitySteps;
			Status = EStationStatus::Ready;
			[[fallthrough]];
		}
		case EStationStatus::Ready:
		{
			ExecuteNextActivity(InInstigator);
			break;
		}
		case EStationStatus::Busy:
		{
			CachedActivitySteps[ActivityIndex]->InteractWhileProcess();
			break;
		}
	}
}

void AStationActorBase::OnActivityFinished(const FActivityOutput& ActivityOutput)
{
	UE_LOG(MS_StationActorBase, Verbose, TEXT("Activity Complete"));

	Status = EStationStatus::Ready;
	if (ActivityOutput.ActivityResult == EActivityResult::Success)
	{
		ExecuteNextActivity(nullptr);
	}
	else
	{
		ResetCurrentActivities();
	}
}

void AStationActorBase::ResetCurrentActivities()
{
	CachedActivitySteps.Reset();
	ActivityIndex = -1;
	Status = EStationStatus::Idle;
}

void AStationActorBase::ExecuteNextActivity(AActor* InInstigator)
{
	++ActivityIndex;

	if (ActivityIndex == CachedActivitySteps.Num())
	{
		UE_LOG(MS_StationActorBase, Error, TEXT("Not implemented"));
		ResetCurrentActivities();
		// TODO Change item asset or delete it
	}
	else if (InInstigator || !CachedActivitySteps[ActivityIndex]->RequiresPlayerInteraction())
	{
		FActivityContext ActivityContext;
		ActivityContext.Instigator = InInstigator;
		ActivityContext.OnActivityFinished.AddDynamic(this, &AStationActorBase::OnActivityFinished);

		UActivityStep* CurrentActivityStep = CachedActivitySteps[ActivityIndex];
		if (!CurrentActivityStep)
		{
			UE_LOG(MS_StationActorBase, Warning,
			       TEXT("Station '%s' encountered a null interaction. Resetting sequence."), *GetName());
			ResetCurrentActivities();
			return;
		}

		Status = EStationStatus::Busy;
		CurrentActivityStep->StartActivity(ActivityContext);
	}
	else
	{
		--ActivityIndex;
	}
}
