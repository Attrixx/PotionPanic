#include "Core/StationActor.h"
#include "AbilitySystemComponent.h"
#include "RecipeSystem/Recipe.h"
#include "RecipeSystem/StationComponent.h"
#include "Core/GameplayAbilitySystem/PotionPanicTags.h"
#include "Core/SocketComponent.h"
#include "Core/SpawnerComponent.h"
#include "Core/SocketableComponent.h"
#include "Core/CamTargetComponent.h"
#include "Core/GameplayAbilitySystem/Abilities/TransformProcessAbility.h"
#include "UserInterface/StationWidget.h"

#include "Components/WidgetComponent.h"

AStationActor::AStationActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    RootComponent = MeshComponent;

	SocketComponent = CreateDefaultSubobject<USocketComponent>(TEXT("SocketComponent"));
	SocketComponent->SetupAttachment(RootComponent);

	StationComponent = CreateDefaultSubobject<UStationComponent>(TEXT("StationComponent"));

	SpawnerComponent = CreateDefaultSubobject<USpawnerComponent>(TEXT("SpawnerComponent"));

	CamTargetComponent = CreateDefaultSubobject<UCamTargetComponent>(TEXT("CamTargetComponent"));

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void AStationActor::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        GetAbilitySystemComponent()->InitAbilityActorInfo(this, this);
        GiveStartupAbilities();
    }

    SocketComponent->OnHeldChanged.AddUObject(this, &AStationActor::OnHeldChanged);
	AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &AStationActor::OnAbilityEnded);
}

void AStationActor::GiveStartupAbilities()
{
    if (!IsValid(GetAbilitySystemComponent())) return;

    for (const auto& Ability : StartupAbilities)
    {
        FGameplayAbilitySpec Spec = FGameplayAbilitySpec(Ability);
        GetAbilitySystemComponent()->GiveAbility(Spec);
    }
}

FRecipe* AStationActor::FindMatchingRecipe(const FInputItemGroup& Items) const
{
    if (!RecipesDataTable)
    {
        return nullptr;
    }

    TArray<FRecipe*> AllRecipes;
    RecipesDataTable->GetAllRows<FRecipe>(TEXT("RecipeSearch"), AllRecipes);

    for (FRecipe* Recipe : AllRecipes)
    {
        if (!Recipe) continue;

        if (!StationTag.IsValid() || !Recipe->Stations.Contains(StationTag)) continue;
        if (StationTag.MatchesTag(PotionPanicTags::Stations::Spawner))  return Recipe;

        if (Recipe->Ingredients.Num() != Items.Counts.Num()) continue;

        bool bMatch = true;
        for (const auto& RecipePair : Recipe->Ingredients)
        {
            const TSubclassOf<AActor>& IngredientClass = RecipePair.Key;
            int32 RequiredAmount = RecipePair.Value;

            const int32* FoundAmountPtr = Items.Counts.Find(IngredientClass);

            if (!FoundAmountPtr || *FoundAmountPtr != RequiredAmount)
            {
                bMatch = false;
                break;
            }
        }

        if (bMatch)
        {
            return Recipe;
        }
    }

    return nullptr;
}

void AStationActor::OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
    if (HasAuthority())
    {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("Ability Ended - Spawning Item"));
        if (CurrentRecipe == nullptr || CurrentRecipe->Product == nullptr) return;
		bDestroySpawnedItem = false;
        SpawnerComponent->SpawnItem(CurrentProcessInstigator, CurrentRecipe->Product);
		bDestroySpawnedItem = true;
    }
}

void AStationActor::OnHeldChanged(USocketableComponent* OldHeld, USocketableComponent* NewHeld)
{
    if (OldHeld == nullptr && NewHeld != nullptr)
    {
        // Picked up an item
        Store();
    }
}

void AStationActor::Store()
{
    if (!SocketComponent->IsHolding())
        return;

    USocketableComponent* ItemComponent = SocketComponent->Take();
    AActor* Item = ItemComponent->GetOwner();

    StationComponent->Store(Item->GetClass());
    if (bDestroySpawnedItem)
    {
        Item->Destroy();
    }
}

void AStationActor::StartProcessing(APawn* ProcessInstigator, FInputItemGroup& Items)
{
    if (!IsValid(AbilitySystemComponent)) return;

	FRecipe* MatchingRecipe = FindMatchingRecipe(Items);
    if (MatchingRecipe == nullptr) return;

	CurrentRecipe = MatchingRecipe;
	CurrentProcessInstigator = ProcessInstigator;
    if (AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(PotionPanicTags::Abilities::TransformItem)))
    {
        Items.Clear();
    }
}

void AStationActor::ShowInteractionUI(bool bShow)
{
    if (!IsValid(WidgetComponent)) return;
    if (UStationWidget* StationWidget = Cast<UStationWidget>(WidgetComponent->GetWidget()))
    {
        if (bShow) StationWidget->ShowInteractKey();
		else StationWidget->HideInteractKey();
    }
}

void AStationActor::ShowAnimatedProgress(float Duration, bool bAutoHide)
{
    if (!IsValid(WidgetComponent)) return;
    if (UStationWidget* StationWidget = Cast<UStationWidget>(WidgetComponent->GetWidget()))
    {
        StationWidget->ShowAnimatedProgress(Duration, bAutoHide);
    }
}

void AStationActor::UpdateProgressUI(float Progress)
{
    if (!IsValid(WidgetComponent)) return;
    if (UStationWidget* StationWidget = Cast<UStationWidget>(WidgetComponent->GetWidget()))
    {
		StationWidget->ShowProgress(Progress);
    }
}

void AStationActor::HideProgressUI()
{
    if (!IsValid(WidgetComponent)) return;
    if (UStationWidget* StationWidget = Cast<UStationWidget>(WidgetComponent->GetWidget()))
    {
        StationWidget->HideProgress();
    }
}
