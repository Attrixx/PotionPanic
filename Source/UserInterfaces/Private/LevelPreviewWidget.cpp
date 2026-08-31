// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelPreviewWidget.h"

#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "LevelHolographicProjectionActor.h"
#include "LobbyGameState.h"

void ULevelPreviewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* LobbyGameState = World->GetGameState<ALobbyGameState>())
		{
			if (ALevelHolographicProjectionActor* Hologram = LobbyGameState->GetLevelHolographicProjection())
			{
				Hologram->OnLevelDataChanged.AddDynamic(this, &ULevelPreviewWidget::HandleLevelDataChanged);
				SetLevelData(Hologram->GetCurrentLevelData());
			}
		}
	}
}

void ULevelPreviewWidget::HandleLevelDataChanged(const FLevelData& NewData)
{
	SetLevelData(NewData);
}

void ULevelPreviewWidget::SetLevelData(const FLevelData& InLevelData)
{
	if (LevelNumberText)
	{
		LevelNumberText->SetText(FText::FromString(FString::Printf(TEXT("Level %d"), InLevelData.LevelNumber)));
	}
	if (LevelNameText)
	{
		LevelNameText->SetText(FText::FromString(InLevelData.LevelName));
	}
	if (LevelScoreText)
	{
		LevelScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %.2f"), InLevelData.Score)));
	}
	if (LevelImageBorder && InLevelData.LevelTexture)
	{
		if (UMaterialInstanceDynamic* DynamicMaterial = LevelImageBorder->GetDynamicMaterial())
		{
			DynamicMaterial->SetTextureParameterValue(FName("LevelImage"), InLevelData.LevelTexture);
		}
	}
}