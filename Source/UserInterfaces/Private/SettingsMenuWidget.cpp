// Fill out your copyright notice in the Description page of Project Settings.

#include "SettingsMenuWidget.h"

#include "Components/WidgetSwitcher.h"
#include "GameFramework/GameUserSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply USettingsMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bIsListeningForInput && RebindingActionName != NAME_None)
	{
		const FKey PressedKey = InKeyEvent.GetKey();

		// Ignore modifier keys alone (Shift, Ctrl, Alt...)
		if (!PressedKey.IsModifierKey())
		{
			const FName ActionName  = RebindingActionName;
			bIsListeningForInput    = false;
			RebindingActionName     = NAME_None;

			OnKeyRebound(ActionName, PressedKey);
			return FReply::Handled();
		}
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void USettingsMenuWidget::BackToPauseMenu()
{
	OnBackClicked.Broadcast();
}

// ── Video ─────────────────────────────────────────────────────────────────────

void USettingsMenuWidget::ApplyVideoSettings(int32 ScreenMode, int32 Quality, int32 Resolution, int32 FPS, int32 VSync)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings) return;

	const EWindowMode::Type ModeMap[] = {
		EWindowMode::Fullscreen,
		EWindowMode::WindowedFullscreen,
		EWindowMode::Windowed
	};
	if (ScreenMode >= 0 && ScreenMode < 3)
	{
		Settings->SetFullscreenMode(ModeMap[ScreenMode]);
	}

	// Quality : 0=Low, 1=Medium, 2=High
	Settings->SetOverallScalabilityLevel(FMath::Clamp(Quality, 0, 2));

	// Resolution — matches Blueprint array order
	const TArray<FIntPoint> Resolutions = { {1280,720}, {1920,1080}, {2560,1440}, {3840,2160} };
	if (Resolutions.IsValidIndex(Resolution))
	{
		Settings->SetScreenResolution(Resolutions[Resolution]);
	}

	// FPS — matches Blueprint array order, 0 = unlimited
	const TArray<int32> FPSValues = { 30, 60, 120, 0 };
	if (FPSValues.IsValidIndex(FPS))
	{
		Settings->SetFrameRateLimit(static_cast<float>(FPSValues[FPS]));
	}

	// VSync : 0=On, 1=Off
	Settings->SetVSyncEnabled(VSync == 0);

	Settings->ApplySettings(false);
	Settings->SaveSettings();
}

// ── Audio ─────────────────────────────────────────────────────────────────────

void USettingsMenuWidget::ApplyAudioSettings(float Master, float Music, float Effects)
{
	if (!MasterSoundMix) return;

	UWorld* World = GetWorld();
	if (!World) return;

	if (MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, MasterSoundClass, Master, 1.0f, 0.0f);
	}
	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, MusicSoundClass, Music, 1.0f, 0.0f);
	}
	if (EffectsSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, EffectsSoundClass, Effects, 1.0f, 0.0f);
	}

	UGameplayStatics::PushSoundMixModifier(World, MasterSoundMix);
}

// ── Controls ──────────────────────────────────────────────────────────────────

void USettingsMenuWidget::StartRebinding(FName ActionName)
{
	RebindingActionName  = ActionName;
	bIsListeningForInput = true;
	SetKeyboardFocus();
}

void USettingsMenuWidget::CancelRebinding()
{
	bIsListeningForInput = false;
	RebindingActionName  = NAME_None;
}

void USettingsMenuWidget::ApplyControlSettings(const TMap<FName, FName>& Mappings)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetOwningLocalPlayer());

	if (!Subsystem) return;

	UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings();
	if (!UserSettings) return;

	for (const TPair<FName, FName>& Pair : Mappings)
	{
		FMapPlayerKeyArgs Args;
		Args.MappingName = Pair.Key;
		Args.NewKey      = FKey(Pair.Value);

		FGameplayTagContainer FailureReason;
		UserSettings->MapPlayerKey(Args, FailureReason);
	}

	UserSettings->AsyncSaveSettings();
}
