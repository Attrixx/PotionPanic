// Fill out your copyright notice in the Description page of Project Settings.

#include "ScorePopupWidget.h"
#include "Components/TextBlock.h"

void UScorePopupWidget::Show(int32 Score)
{
	ScoreText->SetText(FText::Format(NSLOCTEXT("ScorePopup", "PlusPoints", "+{0}"), FText::AsNumber(Score)));
	OnShown(Score);
}
