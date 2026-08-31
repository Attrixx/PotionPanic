#pragma once

#include "CoreMinimal.h"
#include "PotionPanicActivatableWidget.h"
#include "AudioSettingsWidget.generated.h"

class USoundClass;
class USoundMix;

UCLASS()
class USERINTERFACES_API UAudioSettingsWidget : public UPotionPanicActivatableWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeOnInitialized() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMaster(float Value) { Master = FMath::Clamp(Value, 0.0f, 1.0f); bDirty = true; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetMusic(float Value) { Music = FMath::Clamp(Value, 0.0f, 1.0f); bDirty = true; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void SetEffects(float Value) { Effects = FMath::Clamp(Value, 0.0f, 1.0f); bDirty = true; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	float GetMaster() const { return Master; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	float GetMusic() const { return Music; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Settings|Audio")
	float GetEffects() const { return Effects; }

	UFUNCTION(BlueprintCallable, Category = "Settings|Audio")
	void LoadCurrentSettings();

	void ApplyIfDirty();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundMix> MasterSoundMix;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MasterSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Audio")
	TObjectPtr<USoundClass> EffectsSoundClass;

private:

	void Apply();

	float Master  = 1.0f;
	float Music   = 1.0f;
	float Effects = 1.0f;

	bool bDirty = false;
};
