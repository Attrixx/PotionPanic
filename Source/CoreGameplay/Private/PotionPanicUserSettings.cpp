#include "PotionPanicUserSettings.h"

UPotionPanicUserSettings* UPotionPanicUserSettings::GetPotionPanicUserSettings()
{
	return Cast<UPotionPanicUserSettings>(UGameUserSettings::GetGameUserSettings());
}
