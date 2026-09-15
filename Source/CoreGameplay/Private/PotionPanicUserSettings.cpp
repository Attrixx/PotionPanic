#include "PotionPanicUserSettings.h"

UPotionPanicUserSettings* UPotionPanicUserSettings::GetPotionPanicUserSettings()
{
	return Cast<UPotionPanicUserSettings>(UGameUserSettings::GetGameUserSettings());
}

void UPotionPanicUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	MasterVolume = DefaultMasterVolume;
	MusicVolume  = DefaultMusicVolume;
	SFXVolume    = DefaultSFXVolume;

	bVideoDefaultsInitialized = false;
	bAudioDefaultsInitialized = false;
}

void UPotionPanicUserSettings::EnsureVideoDefaults()
{
	if (bVideoDefaultsInitialized) return;

	RunHardwareBenchmark();
	ApplyHardwareBenchmarkResults();
	ApplySettings(false);

	bVideoDefaultsInitialized = true;
	SaveSettings();
}
