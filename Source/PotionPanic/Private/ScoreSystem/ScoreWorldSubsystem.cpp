#include "ScoreSystem/ScoreWorldSubsystem.h"

void UScoreWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    TotalScore = 0;
}

void UScoreWorldSubsystem::AddScore(int32 Points)
{
    if (Points == 0)
    {
        return;
    }

    TotalScore += Points;
    TotalScore = FMath::Max(TotalScore, 0);

    BroadcastScoreChanged();
}

void UScoreWorldSubsystem::SetScore(int32 NewScore)
{
    if (TotalScore == NewScore)
    {
        return;
    }

    TotalScore = NewScore;
    BroadcastScoreChanged();
}

void UScoreWorldSubsystem::ResetScore()
{
    if (TotalScore == 0)
    {
        return;
    }

    TotalScore = 0;
    BroadcastScoreChanged();
}

void UScoreWorldSubsystem::BroadcastScoreChanged()
{
    OnScoreChanged.Broadcast(TotalScore);
}