#include "AudioSettingsWidget.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "Kismet/GameplayStatics.h"

static const FString AudioSaveSlot = TEXT("AudioSettings");

void UAudioSettingsWidget::LoadCurrentSettings()
{
	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(AudioSaveSlot, 0);
	if (!Loaded) return;

	UAudioSaveGame* SaveGame = Cast<UAudioSaveGame>(Loaded);
	if (!SaveGame) return;

	Master  = SaveGame->MasterVolume;
	Music   = SaveGame->MusicVolume;
	Effects = SaveGame->EffectsVolume;
	bDirty  = false;
}

void UAudioSettingsWidget::ApplyIfDirty()
{
	if (!bDirty) return;
	Apply();
	bDirty = false;
}

void UAudioSettingsWidget::Apply()
{
	UAudioSaveGame* SaveGame = NewObject<UAudioSaveGame>();
	SaveGame->MasterVolume  = Master;
	SaveGame->MusicVolume   = Music;
	SaveGame->EffectsVolume = Effects;
	UGameplayStatics::SaveGameToSlot(SaveGame, AudioSaveSlot, 0);

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