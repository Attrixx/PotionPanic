// Copyright 2025


#include "UserInterface/EndMenuWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UEndMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Replay)
	{
		Button_Replay->OnClicked.AddDynamic(this, &UEndMenuWidget::HandleReplayClicked);
	}

	if (Button_MainMenu)
	{
		Button_MainMenu->OnClicked.AddDynamic(this, &UEndMenuWidget::HandleMainMenuClicked);
	}

	if (Text_ReplayLabel)
	{
		Text_ReplayLabel->SetText(ReplayButtonText);
	}

	if (Text_MainMenuLabel)
	{
		Text_MainMenuLabel->SetText(MainMenuButtonText);
	}
}

void UEndMenuWidget::NativeDestruct()
{
	if (Button_Replay)
	{
		Button_Replay->OnClicked.RemoveDynamic(this, &UEndMenuWidget::HandleReplayClicked);
	}

	if (Button_MainMenu)
	{
		Button_MainMenu->OnClicked.RemoveDynamic(this, &UEndMenuWidget::HandleMainMenuClicked);
	}

	Super::NativeDestruct();
}

void UEndMenuWidget::SetEndState(bool bIsVictory, int32 Score)
{
	if (Text_Result)
	{
		Text_Result->SetText(bIsVictory ? VictoryText : DefeatText);
	}

	if (Text_Score)
	{
		const FText ScoreText = FText::Format(FText::FromString(TEXT("Score : {0}")), FText::AsNumber(Score));
		Text_Score->SetText(ScoreText);
	}
}

void UEndMenuWidget::HandleReplayClicked()
{
	OnReplayRequested.Broadcast();
}

void UEndMenuWidget::HandleMainMenuClicked()
{
	OnReturnToMenuRequested.Broadcast();
}
