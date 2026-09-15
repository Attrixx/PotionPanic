// Fill out your copyright notice in the Description page of Project Settings.

#include "PotionPanicPlayerState.h"
#include "AlchemistBase.h"

#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPPCustomization, Log, All);

APotionPanicPlayerState::APotionPanicPlayerState()
{
	// Default customization data asset. Designers can override it on a Blueprint subclass of this
	// PlayerState if the asset is ever moved.
	static ConstructorHelpers::FObjectFinder<UAlchemistCustomizationAsset> CustomizationFinder(
		TEXT("/Game/Data/Player/DA_AlchemistCustomization.DA_AlchemistCustomization"));
	if (CustomizationFinder.Succeeded())
	{
		CustomizationAsset = CustomizationFinder.Object;
	}
}

void APotionPanicPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APotionPanicPlayerState, CustomizationColor);
	DOREPLIFETIME(APotionPanicPlayerState, PlayerSlotIndex);
}

void APotionPanicPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	if (APotionPanicPlayerState* NewPS = Cast<APotionPanicPlayerState>(NewPlayerState))
	{
		NewPS->CustomizationColor = CustomizationColor;
		NewPS->PlayerSlotIndex = PlayerSlotIndex;

		UE_LOG(LogPPCustomization, Warning,
			TEXT("[CopyProperties] %s -> %s : Color=%d Slot=%d"),
			*GetClass()->GetName(), *NewPS->GetClass()->GetName(),
			(int32)CustomizationColor, PlayerSlotIndex);
	}
	else
	{
		UE_LOG(LogPPCustomization, Error,
			TEXT("[CopyProperties] %s -> %s : target is NOT an APotionPanicPlayerState, customization lost"),
			*GetClass()->GetName(),
			NewPlayerState ? *NewPlayerState->GetClass()->GetName() : TEXT("null"));
	}
}

void APotionPanicPlayerState::BeginPlay()
{
	Super::BeginPlay();

	OnPawnSet.AddDynamic(this, &APotionPanicPlayerState::HandlePotionPanicPawnSet);

	UE_LOG(LogPPCustomization, Warning,
		TEXT("[BeginPlay] %s HasAuthority=%d Color=%d Slot=%d Asset=%s AutoApply=%d"),
		*GetClass()->GetName(), HasAuthority(), (int32)CustomizationColor, PlayerSlotIndex,
		*GetNameSafe(CustomizationAsset), ShouldAutoApplyCustomization());

	// The pawn may already be possessed (e.g. after seamless travel); apply immediately.
	OnCustomizationDataChanged();
}

void APotionPanicPlayerState::SetCustomizationColor(EAlchemistColor NewColor)
{
	CustomizationColor = NewColor;
	OnCustomizationDataChanged();
}

void APotionPanicPlayerState::SetPlayerSlotIndex(int32 NewIndex)
{
	PlayerSlotIndex = NewIndex;
	OnCustomizationDataChanged();
}

FColor APotionPanicPlayerState::GetResolvedColor() const
{
	return IsValid(CustomizationAsset) ? CustomizationAsset->GetColor(CustomizationColor) : FColor::White;
}

void APotionPanicPlayerState::GetCustomizationVisuals(USkeletalMesh*& OutMesh, FColor& OutColor) const
{
	OutMesh = nullptr;
	OutColor = FColor::White;

	if (IsValid(CustomizationAsset))
	{
		OutMesh = CustomizationAsset->GetMesh(CustomizationColor);
		OutColor = CustomizationAsset->GetColor(CustomizationColor);
	}
}

void APotionPanicPlayerState::ApplyCustomizationToCharacter(AAlchemistBase* Character) const
{
	if (!IsValid(Character)) return;

	USkeletalMesh* MeshToUse = nullptr;
	FColor ColorToUse = FColor::White;
	GetCustomizationVisuals(MeshToUse, ColorToUse);

	UE_LOG(LogPPCustomization, Warning,
		TEXT("[ApplyCustomization] %s -> pawn %s : Color=%d Mesh=%s Asset=%s"),
		*GetClass()->GetName(), *Character->GetName(),
		(int32)CustomizationColor, *GetNameSafe(MeshToUse), *GetNameSafe(CustomizationAsset));

	Character->ApplyCustomization(MeshToUse, ColorToUse);

	// Convention shared with the Lobby: stencil 0 = disabled, 1-4 identify players.
	Character->SetPlayerStencilIndex(PlayerSlotIndex + 1);
}

void APotionPanicPlayerState::OnCustomizationDataChanged()
{
	if (!ShouldAutoApplyCustomization()) return;

	if (AAlchemistBase* Character = Cast<AAlchemistBase>(GetPawn()))
	{
		ApplyCustomizationToCharacter(Character);
	}
}

void APotionPanicPlayerState::OnRep_CustomizationColor()
{
	OnCustomizationDataChanged();
}

void APotionPanicPlayerState::OnRep_PlayerSlotIndex()
{
	OnCustomizationDataChanged();
}

void APotionPanicPlayerState::HandlePotionPanicPawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	UE_LOG(LogPPCustomization, Warning,
		TEXT("[PawnSet] %s NewPawn=%s AutoApply=%d Color=%d"),
		*GetClass()->GetName(), *GetNameSafe(NewPawn), ShouldAutoApplyCustomization(), (int32)CustomizationColor);

	if (!ShouldAutoApplyCustomization()) return;

	if (AAlchemistBase* Character = Cast<AAlchemistBase>(NewPawn))
	{
		ApplyCustomizationToCharacter(Character);
	}
}
