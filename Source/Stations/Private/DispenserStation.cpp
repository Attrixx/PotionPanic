#include "DispenserStation.h"
#include "ItemAsset.h"
#include "ItemActor.h"
#include "CarriableComponent.h"
#include "HolderComponent.h" 
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"

DEFINE_LOG_CATEGORY_STATIC(LogDispenserStation, Log, All);

ADispenserStation::ADispenserStation()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADispenserStation::Interact(APlayerController& InInstigator)
{
	Super::Interact(InInstigator);

	if (!IsValid(IngredientToDispense))
	{
		UE_LOGFMT(LogDispenserStation, Warning, "DispenserStation {0} has no IngredientToDispense set!", GetName());
		return;
	}

	APawn* Pawn = InInstigator.GetPawn();
	if (!IsValid(Pawn))
	{
		return;
	}

	UHolderComponent* Holder = Pawn->FindComponentByClass<UHolderComponent>();
	if (!IsValid(Holder))
	{
		UE_LOGFMT(LogDispenserStation, Warning, "Instigator {0} has no HolderComponent!", Pawn->GetName());
		return;
	}

	if (Holder->GetCarriable() != nullptr)
	{
		// TODO (Nath) : Add visual/audio feedback for "Hands Full"
		UE_LOGFMT(LogDispenserStation, Verbose, "Instigator hands are full, cannot dispense.");
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AItemActor* NewItem = GetWorld()->SpawnActor<AItemActor>(AItemActor::StaticClass(), FTransform::Identity, SpawnParams);
	
	if (IsValid(NewItem))
	{
		NewItem->SetItemAsset(IngredientToDispense);
		
		UCarriableComponent* NewCarriable = NewItem->FindComponentByClass<UCarriableComponent>();
		if (IsValid(NewCarriable))
		{
			Holder->Replace(NewCarriable);
			UE_LOGFMT(LogDispenserStation, Log, "Dispensed {0} to {1}", IngredientToDispense->GetName(), Pawn->GetName());
		}
		else
		{
			UE_LOGFMT(LogDispenserStation, Error, "Spawned Item {0} has no CarriableComponent!", NewItem->GetName());
			NewItem->Destroy();
		}
	}
}
