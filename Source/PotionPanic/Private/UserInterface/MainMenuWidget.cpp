// Copyright 2025

#include "UserInterface/MainMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Play)
	{
		Button_Play->OnClicked.AddDynamic(this, &UMainMenuWidget::HandlePlayClicked);
	}

	if (Text_PlayLabel)
	{
		Text_PlayLabel->SetText(PlayButtonText);
	}
}

void UMainMenuWidget::NativeDestruct()
{
    if (Button_Play)
    {
        Button_Play->OnClicked.RemoveDynamic(this, &UMainMenuWidget::HandlePlayClicked);
    }

    Super::NativeDestruct();
}

void UMainMenuWidget::HandlePlayClicked()
{
    OnPlayRequested.Broadcast();
}
