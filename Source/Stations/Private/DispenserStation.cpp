#include "DispenserStation.h"
#include "ItemAsset.h"
#include "ItemActor.h"
#include "CarriableComponent.h"
#include "HolderComponent.h" 
#include "GameFramework/PlayerController.h"
#include "Logging/StructuredLog.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

DEFINE_LOG_CATEGORY_STATIC(MS_DispenserStation, Log, All);

ADispenserStation::ADispenserStation()
{
}

void ADispenserStation::Interact(APlayerController& InInstigator)
{
	Super::Interact(InInstigator);

	if (!IngredientToDispense.IsValid())
	{
		UE_LOGFMT(MS_DispenserStation, Warning, "DispenserStation {0} has no IngredientToDispense set!", GetName());
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
		UE_LOGFMT(MS_DispenserStation, Warning, "Instigator {0} has no HolderComponent!", Pawn->GetName());
		return;
	}

	if (Holder->GetCarriable() != nullptr)
	{
		// TODO (Nath) : Add visual/audio feedback for "Hands Full"
		UE_LOGFMT(MS_DispenserStation, Verbose, "Instigator hands are full, cannot dispense.");
		return;
	}

	// Load Asset Synchronously for spawning
	UItemAsset* ItemAsset = Cast<UItemAsset>(UAssetManager::GetStreamableManager().LoadSynchronous(IngredientToDispense));
	if (!ItemAsset)
	{
		UE_LOGFMT(MS_DispenserStation, Error, "Failed to load ItemAsset from ID {0}", IngredientToDispense.ToString());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AItemActor* NewItem = GetWorld()->SpawnActor<AItemActor>(AItemActor::StaticClass(), FTransform::Identity, SpawnParams);
	
	if (IsValid(NewItem))
	{
		NewItem->SetItemAsset(ItemAsset);
		
		UCarriableComponent* NewCarriable = NewItem->FindComponentByClass<UCarriableComponent>();
		if (IsValid(NewCarriable))
		{
			Holder->Replace(NewCarriable);
			UE_LOGFMT(MS_DispenserStation, Log, "Dispensed {0} to {1}", ItemAsset->GetName(), Pawn->GetName());
		}
		else
		{
			UE_LOGFMT(MS_DispenserStation, Error, "Spawned Item {0} has no CarriableComponent!", NewItem->GetName());
			NewItem->Destroy();
		}
	}
}
