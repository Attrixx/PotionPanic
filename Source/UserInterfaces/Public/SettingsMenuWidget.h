// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UWidgetSwitcher;
class USoundClass;
class USoundMix;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackClicked);

UCLASS()
class USERINTERFACES_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:

	/** Fired when the user clicks back. Bind this in WBP_InGameMenu Blueprint. */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Settings")
	FOnBackClicked OnBackClicked;

	/** Called by Blueprint back button. Broadcasts OnBackClicked. */
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void BackToPauseMenu();

	// ── Video ────────────────────────────────────────────────────────────────

	/**
	 * Called by WBP_VideoSettings Blueprint when the user confirms settings.
	 * Indices must match the order of options in each WBP_OptionCycle Blueprint array.
	 * ScreenMode : 0=Fullscreen, 1=Borderless, 2=Windowed
	 * Quality    : 0=Low, 1=Medium, 2=High
	 * Resolution : 0=1280x720, 1=1920x1080, 2=2560x1440, 3=3840x2160
	 * FPS        : 0=30, 1=60, 2=120, 3=Unlimited
	 * VSync      : 0=On, 1=Off
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Video")
	void ApplyVideoSettings(int32 ScreenMode, int32 Quality, int32 Resolution, int32 FPS, int32 VSync);

	// ── Audio ────────────────────────────────────────────────────────────────

	/**
	 * Called by WBP_SoundSettings Blueprint when the user confirms settings.
	 * Values are in range 0.0 - 1.0, matching the slider values directly.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void ApplyAudioSettings(float Master, float Music, float Effects);

	// ── Controls ─────────────────────────────────────────────────────────────

	/**
	 * Enters "waiting for key press" mode for the given action.
	 * Call this from WBP_OptionKey when the player clicks on a binding.
	 * ActionName must match the FName key in your InputMappings Blueprint map.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void StartRebinding(FName ActionName);

	/** Cancels the current rebinding operation (e.g. player pressed Escape). */
	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void CancelRebinding();

	/**
	 * Applies all control mappings via Enhanced Input.
	 * Mappings: ActionName (FName) → KeyName (FName), matches your Blueprint TMap.
	 * Requires Input Actions to have PlayerMappableKeySettings set.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Controls")
	void ApplyControlSettings(const TMap<FName, FName>& Mappings);

	/**
	 * Implemented in Blueprint (WBP_ControlsSetting).
	 * Called when a key has been successfully captured — use it to update the UI.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Settings|Controls")
	void OnKeyRebound(FName ActionName, FKey NewKey);

protected:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WS_ContentSwitcher;

	// ── Audio assets — assign in WBP_SettingsMenu Blueprint defaults ──────────

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundMix> MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> EffectsSoundClass;

	// ── Controls — assign in WBP_SettingsMenu Blueprint defaults ─────────────

	/** The InputMappingContext that contains the player-mappable actions. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings|Controls")
	TObjectPtr<UInputMappingContext> InputMappingContext;

private:

	bool  bIsListeningForInput = false;
	FName RebindingActionName  = NAME_None;
};
