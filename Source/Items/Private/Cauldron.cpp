#include "Cauldron.h"
#include "CarriableComponent.h"
#include "IngredientActor.h"
#include "ItemActor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "HolderComponent.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY_STATIC(MS_Cauldron, Log, All);

ACauldron::ACauldron()
{
	bReplicates = true;
}

void ACauldron::Interact(APlayerController& InInstigator)
{
	if (!HasAuthority())
	{
		return;
	}

	APawn* InstigatorPawn = InInstigator.GetPawn();
	if (InstigatorPawn == nullptr)
	{
		return;
	}

	UHolderComponent* PlayerHolder = InstigatorPawn->FindComponentByClass<UHolderComponent>();
	if (PlayerHolder == nullptr || Carriable == nullptr)
	{
		return;
	}

	UCarriableComponent* PlayerCarriable = PlayerHolder->GetCarriable();
	if (PlayerCarriable && PlayerCarriable != Carriable)
	{
		AItemActor* HeldItemActor = Cast<AItemActor>(PlayerCarriable->GetOwner());
		if (!HeldItemActor || !HeldItemActor->IsA<AIngredientActor>())
		{
			return;
		}

		if (AddIngredientAssetId(PlayerCarriable->GetItemId()))
		{
			PlayerHolder->Replace(nullptr);
			HeldItemActor->DestroyItem(true);
		}
		return;
	}

	if (PlayerCarriable == Carriable)
	{
		// Drop cauldron from player's hands.
		PlayerHolder->Replace(nullptr);
		return;
	}

	if (UHolderComponent* CurrentHolder = Carriable->GetHolder())
	{
		CurrentHolder->Replace(nullptr);
	}

	PlayerHolder->Replace(Carriable);
}

void ACauldron::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACauldron, IngredientContents);
	DOREPLIFETIME(ACauldron, FillRatio);
	DOREPLIFETIME(ACauldron, LiquidTint);
	DOREPLIFETIME(ACauldron, VisualFlags);
}

bool ACauldron::AddIngredientAssetId(FPrimaryAssetId IngredientAssetId)
{
	if (!HasAuthority() || !IngredientAssetId.IsValid())
	{
		return false;
	}

	IngredientContents.Add(IngredientAssetId);
	FillRatio = FMath::Clamp(static_cast<float>(IngredientContents.Num()) / FMath::Max(1, MaxIngredientVisualCount), 0.0f, 1.0f);

	OnContentsChangedBP();
	OnVisualStateChangedBP();
	return true;
}

void ACauldron::ClearIngredients()
{
	if (!HasAuthority())
	{
		return;
	}

	IngredientContents.Reset();
	FillRatio = 0.0f;
	OnContentsChangedBP();
	OnVisualStateChangedBP();
}

void ACauldron::SetFillRatio(float NewFillRatio)
{
	if (!HasAuthority())
	{
		return;
	}

	FillRatio = FMath::Clamp(NewFillRatio, 0.0f, 1.0f);
	OnVisualStateChangedBP();
}

void ACauldron::SetLiquidTint(FLinearColor NewTint)
{
	if (!HasAuthority())
	{
		return;
	}

	LiquidTint = NewTint;
	OnVisualStateChangedBP();
}

void ACauldron::SetVisualFlags(const TArray<FName>& NewFlags)
{
	if (!HasAuthority())
	{
		return;
	}

	VisualFlags = NewFlags;
	OnVisualStateChangedBP();
}

void ACauldron::OnRep_IngredientContents()
{
	OnContentsChangedBP();
}

void ACauldron::OnRep_VisualState()
{
	OnVisualStateChangedBP();
}
