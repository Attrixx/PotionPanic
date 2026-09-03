// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AlchemistCustomizationAsset.h"
#include "PotionPanicPlayerState.generated.h"

class AAlchemistBase;

/**
 * Shared PlayerState base for every game mode (Lobby + gameplay levels).
 *
 * Carries the player's visual customization (color / slot) and, because it overrides
 * CopyProperties, that data survives seamless travel: when the player walks through a
 * LevelSelector door the gameplay GameMode spawns a fresh PlayerState of this class and
 * the engine copies the customization from the outgoing Lobby PlayerState into it.
 */
UCLASS()
class PLAYER_API APotionPanicPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	APotionPanicPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called on the server during seamless travel to carry data into the new PlayerState.
	virtual void CopyProperties(APlayerState* NewPlayerState) override;

	// ==================== Customization ====================

	/** Server-only. Sets the customization color, replicates it and re-applies it to the current pawn. */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetCustomizationColor(EAlchemistColor NewColor);

	UFUNCTION(BlueprintCallable, Category = "Customization")
	EAlchemistColor GetCustomizationColor() const { return CustomizationColor; }

	/** Server-only. Player slot (0..3), used for the custom-depth stencil and colored MPC params. */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	void SetPlayerSlotIndex(int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "Customization")
	int32 GetPlayerSlotIndex() const { return PlayerSlotIndex; }

	/** Resolved physical color for the current customization color (White if no data asset). */
	UFUNCTION(BlueprintCallable, Category = "Customization")
	FColor GetResolvedColor() const;

	UAlchemistCustomizationAsset* GetCustomizationAsset() const { return CustomizationAsset; }

	/** Fills the mesh + color to use for the current customization color. */
	void GetCustomizationVisuals(USkeletalMesh*& OutMesh, FColor& OutColor) const;

	/** Applies mesh + color + player stencil to the given character. */
	void ApplyCustomizationToCharacter(AAlchemistBase* Character) const;

protected:

	virtual void BeginPlay() override;

	/**
	 * When true, the base class re-applies the customization to the possessed pawn automatically
	 * (on pawn set and on replication). The Lobby drives its own visuals from FLobbyPlayerInfo, so
	 * it opts out; gameplay levels rely on this.
	 */
	virtual bool ShouldAutoApplyCustomization() const { return true; }

	/** Server + client hook fired whenever the customization color or slot changes. */
	virtual void OnCustomizationDataChanged();

	/** Data asset mapping color enum -> mesh / physical color. Defaulted in the constructor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<UAlchemistCustomizationAsset> CustomizationAsset;

	UPROPERTY(ReplicatedUsing = OnRep_CustomizationColor)
	EAlchemistColor CustomizationColor = EAlchemistColor::Blue;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerSlotIndex)
	int32 PlayerSlotIndex = 0;

	UFUNCTION()
	void OnRep_CustomizationColor();

	UFUNCTION()
	void OnRep_PlayerSlotIndex();

	UFUNCTION()
	void HandlePotionPanicPawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
};
