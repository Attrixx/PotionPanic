#include "CauldronStation.h"
#include "SocketComponent.h"
#include "ItemActor.h"
#include "ItemAsset.h"

ACauldronStation::ACauldronStation()
{
	PrimaryActorTick.bCanEverTick = true;

	CauldronSocket = CreateDefaultSubobject<USocketComponent>(TEXT("CauldronSocket"));
	CauldronSocket->SetupAttachment(RootComponent);
}

void ACauldronStation::Execute(const FInstruction& Instruction)
{
	Super::Execute(Instruction);

	if (bIsTransforming)
	{
		UE_LOG(LogTemp, Warning, TEXT("CauldronStation: Already cooking"));
		return;
	}

	if (Ingredients.Num() >= MaxIngredients)
	{
		UE_LOG(LogTemp, Warning, TEXT("CauldronStation: Maximum ingredients reached"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("CauldronStation: Ingredient added. Total: %d"), Ingredients.Num());

	if (Ingredients.Num() > 0)
	{
		StartTransformation(CookingDuration);
		UE_LOG(LogTemp, Log, TEXT("CauldronStation: Started cooking"));
	}
}

void ACauldronStation::OnTransformationCompleted()
{
	Super::OnTransformationCompleted();

	for (AItemActor* Ingredient : Ingredients)
	{
		if (Ingredient)
		{
			Ingredient->Destroy();
		}
	}
	Ingredients.Empty();

	UE_LOG(LogTemp, Log, TEXT("CauldronStation: Cooking completed"));
}
