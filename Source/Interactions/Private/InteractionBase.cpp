#include "InteractionBase.h"

bool UInteractionBase::StartInteraction(const UInteractionDefinitionAsset* InDefinition)
{
	if (bIsRunning || InDefinition == nullptr)
	{
		return false;
	}

	Definition = InDefinition;
	RuntimeState = FInteractionRuntimeState{};
	bIsRunning = true;
	OnInteractionStateChanged.Broadcast(RuntimeState);
	return true;
}

void UInteractionBase::RegisterAttempt(bool bSuccess)
{
	if (!bIsRunning || Definition == nullptr)
	{
		return;
	}

	++RuntimeState.AttemptCount;
	if (bSuccess)
	{
		++RuntimeState.SuccessCount;
	}
	else
	{
		++RuntimeState.FailureCount;
	}

	OnInteractionStateChanged.Broadcast(RuntimeState);

	if (RuntimeState.SuccessCount >= GetRequiredSuccessCount())
	{
		FinishInteraction(EInteractionResult::Success);
		return;
	}

	if (RuntimeState.FailureCount > GetAllowedFailureCount())
	{
		FinishInteraction(EInteractionResult::Fail);
	}
}

void UInteractionBase::AdvanceTime(float DeltaSeconds)
{
	if (!bIsRunning || Definition == nullptr || DeltaSeconds <= 0.0f)
	{
		return;
	}

	RuntimeState.ElapsedSeconds += DeltaSeconds;
	OnInteractionStateChanged.Broadcast(RuntimeState);

	if (RuntimeState.ElapsedSeconds >= GetMaxDurationSeconds())
	{
		FinishInteraction(EInteractionResult::Timeout);
	}
}

void UInteractionBase::CancelInteraction()
{
	if (!bIsRunning)
	{
		return;
	}

	FinishInteraction(EInteractionResult::Cancelled);
}

void UInteractionBase::FinishInteraction(EInteractionResult Result)
{
	if (!bIsRunning)
	{
		return;
	}

	bIsRunning = false;
	RuntimeState.Result = Result;
	OnInteractionStateChanged.Broadcast(RuntimeState);

	FInteractionOutput Output;
	Output.InteractionResult = Result;
	Output.Score = ComputeScore();
	Output.SuccessCount = RuntimeState.SuccessCount;
	Output.FailureCount = RuntimeState.FailureCount;
	Output.DurationSeconds = RuntimeState.ElapsedSeconds;
	OnInteractionFinished.Broadcast(Output);
}

int32 UInteractionBase::ComputeScore() const
{
	if (Definition == nullptr)
	{
		return 0;
	}

	const FInteractionScoringRule& Scoring = Definition->Scoring;
	int32 Score = Scoring.BaseScore;

	Score += RuntimeState.SuccessCount * GetPointsPerSuccess();
	Score -= RuntimeState.FailureCount * GetPointsPerFailure();

	if (RuntimeState.Result == EInteractionResult::Success)
	{
		Score += Scoring.SuccessBonus;

		if (RuntimeState.FailureCount == 0 && RuntimeState.ElapsedSeconds <= Scoring.PerfectTimeThresholdSeconds)
		{
			Score += Scoring.PerfectBonus;
		}
	}
	else if (RuntimeState.Result == EInteractionResult::Fail || RuntimeState.Result == EInteractionResult::Timeout)
	{
		Score -= Scoring.FailurePenalty;
	}

	return FMath::Max(Scoring.MinScore, Score);
}

int32 UInteractionBase::GetRequiredSuccessCount() const
{
	if (Definition == nullptr)
	{
		return 1;
	}

	return Definition->Rules.RequiredSuccessCount;
}

int32 UInteractionBase::GetAllowedFailureCount() const
{
	if (Definition == nullptr)
	{
		return 0;
	}

	return Definition->Rules.AllowedFailureCount;
}

float UInteractionBase::GetMaxDurationSeconds() const
{
	if (Definition == nullptr)
	{
		return 0.0f;
	}

	return Definition->Rules.MaxDurationSeconds;
}

int32 UInteractionBase::GetPointsPerSuccess() const
{
	if (Definition == nullptr)
	{
		return 0;
	}

	return Definition->Rules.PointsPerSuccess;
}

int32 UInteractionBase::GetPointsPerFailure() const
{
	if (Definition == nullptr)
	{
		return 0;
	}

	return Definition->Rules.PointsPerFailure;
}
