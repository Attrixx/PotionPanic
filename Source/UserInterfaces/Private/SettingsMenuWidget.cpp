#include "SettingsMenuWidget.h"
#include "VideoSettingsWidget.h"
#include "AudioSettingsWidget.h"
#include "ControlsSettingsWidget.h"

void USettingsMenuWidget::ApplyAndClose()
{
	if (WBP_VideoSettings)    WBP_VideoSettings->ApplyIfDirty();
	if (WBP_AudioSettings)    WBP_AudioSettings->ApplyIfDirty();
	if (WBP_ControlsSettings) WBP_ControlsSettings->ApplyIfDirty();

	OnBackClicked.Broadcast();
}