#include "AudioSettingsWidget.h"
#include "PotionPanicUserSettings.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Kismet/GameplayStatics.h"

void UAudioSettingsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	bAutoActivate = false;
	bIsBackHandler = false;
}

static float ReadSoundClassVolume(const USoundClass* SoundClass, float Fallback)
{
	return SoundClass ? SoundClass->Properties.Volume : Fallback;
}

void UAudioSettingsWidget::LoadCurrentSettings()
{
	UPotionPanicUserSettings* Settings = UPotionPanicUserSettings::GetPotionPanicUserSettings();
	if (!Settings) return;

	if (!Settings->bAudioDefaultsInitialized)
	{
		Master  = ReadSoundClassVolume(MasterSoundClass,  Settings->MasterVolume);
		Music   = ReadSoundClassVolume(MusicSoundClass,   Settings->MusicVolume);
		Effects = ReadSoundClassVolume(EffectsSoundClass, Settings->SFXVolume);

		Settings->MasterVolume = Master;
		Settings->MusicVolume  = Music;
		Settings->SFXVolume    = Effects;
		Settings->bAudioDefaultsInitialized = true;
		Settings->SaveSettings();

		Apply();
	}
	else
	{
		Master  = Settings->MasterVolume;
		Music   = Settings->MusicVolume;
		Effects = Settings->SFXVolume;
	}

	bDirty = false;
}

void UAudioSettingsWidget::ApplyIfDirty()
{
	if (!bDirty) return;
	Apply();
	bDirty = false;
}

void UAudioSettingsWidget::Apply()
{
	UPotionPanicUserSettings* Settings = UPotionPanicUserSettings::GetPotionPanicUserSettings();
	if (Settings)
	{
		Settings->MasterVolume = Master;
		Settings->MusicVolume  = Music;
		Settings->SFXVolume    = Effects;
		Settings->SaveSettings();
	}

	UWorld* World = GetWorld();
	if (!World || !MasterSoundMix) return;

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
