#include "Cauldron.h"
#include "ItemAsset.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "CarriableComponent.h"
#include "HolderComponent.h"

ACauldron::ACauldron()
{
	bReplicates = true;
}

void ACauldron::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACauldron, CurrentIngredients);
}

void ACauldron::Interact(APlayerController& InInstigator)
{
	if (!InInstigator.GetPawn()) return;

	UHolderComponent* PlayerHolder = InInstigator.GetPawn()->FindComponentByClass<UHolderComponent>();
	if (PlayerHolder)
	{
		UCarriableComponent* HeldItem = PlayerHolder->GetCarriable();
		if (HeldItem)
		{
			AActor* OwnerActor = HeldItem->GetOwner();
			AItemActor* HeldItemActor = Cast<AItemActor>(OwnerActor);
			
			if (HeldItemActor && !HeldItemActor->IsA<AUtensilActor>())
			{
				if (HeldItemActor->GetItemAsset())
				{
					AddIngredient(HeldItemActor->GetItemAsset());
					OwnerActor->Destroy();
					PlayerHolder->Replace(nullptr);
					return;
				}
			}
			
			if (HeldItemActor && HeldItemActor->IsA<AItemActor>())
			{
				const UItemAsset* HeldAsset = HeldItemActor->GetItemAsset();
				if (HeldAsset && HeldAsset->bIsContainer)
				{
					if (CurrentIngredients.Num() > 0)
					{
						UItemAsset* Potion = CurrentIngredients[0];
						
						OwnerActor->Destroy();
						PlayerHolder->Replace(nullptr);
						
						FActorSpawnParameters SpawnParams;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
						AItemActor* NewPotion = GetWorld()->SpawnActor<AItemActor>(AItemActor::StaticClass(), GetActorTransform(), SpawnParams);
						if (NewPotion)
						{
							NewPotion->SetItemAsset(Potion);
							UCarriableComponent* NewCarriable = NewPotion->FindComponentByClass<UCarriableComponent>();
							if (NewCarriable)
							{
								PlayerHolder->Replace(NewCarriable);
							}
						}
						
						EmptyCauldron();
						return;
					}
				}
			}
		}
	}
}

void ACauldron::AddIngredient(UItemAsset* Ingredient)
{
	if (Ingredient)
	{
		CurrentIngredients.Add(Ingredient);
		// TODO (Nath): Update Visuals
	}
}

void ACauldron::EmptyCauldron()
{
	CurrentIngredients.Empty();
	// TODO (Nath): Update Visuals
}
