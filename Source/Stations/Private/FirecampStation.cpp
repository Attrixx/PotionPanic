#include "FirecampStation.h"
#include "SocketComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"

AFirecampStation::AFirecampStation()
{
	PrimaryActorTick.bCanEverTick = true;

	CauldronSocket = CreateDefaultSubobject<USocketComponent>(TEXT("CauldronSocket"));
	CauldronSocket->SetupAttachment(RootComponent);
}

void AFirecampStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);

	if (bIsTransforming)
	{
		UE_LOG(LogTemp, Warning, TEXT("FirecampStation: Already cooking"));
		return;
	}

	// Check if there's a cauldron placed
	if (!CurrentCauldron)
	{
		UE_LOG(LogTemp, Warning, TEXT("FirecampStation: No cauldron placed on firecamp"));
		return;
	}

	// Check if cauldron has ingredients
	if (!CauldronHasIngredients())
	{
		UE_LOG(LogTemp, Warning, TEXT("FirecampStation: Cauldron is empty, cannot cook"));
		return;
	}

	// Start cooking transformation
	StartTransformation(CookingDuration);
	UE_LOG(LogTemp, Log, TEXT("FirecampStation: Started cooking cauldron contents"));
}

void AFirecampStation::OnTransformationCompleted()
{
	Super::OnTransformationCompleted();

	UE_LOG(LogTemp, Log, TEXT("FirecampStation: Cooking complete!"));
	
	// TODO: Transform cauldron contents into output item
	// This will be handled by the recipe system
}

bool AFirecampStation::CauldronHasIngredients() const
{
	if (!CurrentCauldron)
	{
		return false;
	}

	// Check if the cauldron has any items attached (ingredients)
	TArray<AActor*> AttachedActors;
	CurrentCauldron->GetAttachedActors(AttachedActors);

	// Filter to only count AItemActor children
	for (AActor* Actor : AttachedActors)
	{
		if (Cast<AItemActor>(Actor))
		{
			return true;
		}
	}

	return false;
}
