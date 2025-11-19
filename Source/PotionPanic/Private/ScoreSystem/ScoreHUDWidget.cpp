#include "ScoreSystem/ScoreHUDWidget.h"
#include "Components/TextBlock.h"
#include "ScoreSystem/ScoreWorldSubsystem.h"
#include "Engine/World.h"

void UScoreHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UWorld *World = GetWorld())
    {
        ScoreSubsystem = World->GetSubsystem<UScoreWorldSubsystem>();
    }

    if (ScoreSubsystem)
    {
        ScoreSubsystem->OnScoreChanged.AddDynamic(this, &UScoreHUDWidget::HandleScoreChanged);

        const int32 CurrentScore = ScoreSubsystem->GetScore();
        UpdateScoreText(CurrentScore);
    }
    else
    {

        UpdateScoreText(0);
    }
}

void UScoreHUDWidget::NativeDestruct()
{
    if (ScoreSubsystem)
    {
        ScoreSubsystem->OnScoreChanged.RemoveDynamic(this, &UScoreHUDWidget::HandleScoreChanged);
        ScoreSubsystem = nullptr;
    }

    Super::NativeDestruct();
}

void UScoreHUDWidget::HandleScoreChanged(int32 NewScore)
{
    UpdateScoreText(NewScore);
}

void UScoreHUDWidget::UpdateScoreText(int32 NewScore)
{
    if (!ScoreText)
    {
        return;
    }

    const FText ScoreValueText = FText::AsNumber(NewScore);

    if (!ScorePrefix.IsEmpty())
    {
        FText CombinedText = FText::Format(
            FText::FromString(TEXT("{0}{1}")),
            ScorePrefix,
            ScoreValueText);

        ScoreText->SetText(CombinedText);
    }
    else
    {
        ScoreText->SetText(ScoreValueText);
    }
}
