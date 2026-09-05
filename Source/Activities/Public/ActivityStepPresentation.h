// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "ActivityStepPresentation.generated.h"

class UActivityStep;

/**
 * One of the four inputs an input-driven step can ask for.
 *
 * The names are the directions they read as on screen, not the keys behind them: this module knows
 * nothing about Enhanced Input, and UActivityInputSettings is what binds each one to a real action
 * -- the arrow cluster on a keyboard, the right-hand face buttons on a pad.
 *
 * The values are single bits so a set of allowed inputs is one int32 mask, but an enumerator on its
 * own still names exactly one input: that is how it is used everywhere but in those masks.
 */
UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EActivityInputSlot : uint8
{
	None  = 0 UMETA(Hidden),
	Up    = 1 << 0,
	Left  = 1 << 1,
	Down  = 1 << 2,
	Right = 1 << 3,
};

/** Outcome of a single press in a combo. */
UENUM(BlueprintType)
enum class EActivityPressResult : uint8
{
	Pending = 0,
	Hit,
	Miss,
};

/**
 * Everything a client needs to draw the running step. Replicated to everyone, because a step is
 * watchable by the whole table even when a single player drives it.
 *
 * Times are absolute server times (AGameStateBase::GetServerWorldTimeSeconds), never remaining
 * durations: clients interpolate their own progress bars from them, so a window costs one
 * replication when it opens rather than one per frame.
 */
USTRUCT(BlueprintType)
struct ACTIVITIES_API FActivityStepPresentation
{
	GENERATED_BODY()

	/**
	 * The step being drawn, which is also what picks the widget: UActivityDisplaySettings maps step
	 * classes to widget classes, so a new step only has to add a row there. Null when nothing is
	 * running, and stamped by the executor rather than by the steps themselves.
	 */
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UActivityStep> StepClass = nullptr;

	/**
	 * Actor the step is running for, null when it has no owner yet. Widgets compare it against
	 * their own local player to decide what to draw: the instigator gets the prompts, the others
	 * get whatever the spectator view is.
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> Instigator = nullptr;

	/** Server time this became the thing being drawn. */
	UPROPERTY(BlueprintReadOnly)
	double StartServerTime = 0.0;

	/**
	 * Server time it starts accepting input. Equal to StartServerTime when there is no wait, which
	 * is the case for everything that is askable the moment it appears.
	 */
	UPROPERTY(BlueprintReadOnly)
	double OpenServerTime = 0.0;

	/**
	 * Server time it stops accepting input: the window closes, the press times out.
	 * Equal to OpenServerTime when it never expires.
	 */
	UPROPERTY(BlueprintReadOnly)
	double CloseServerTime = 0.0;

	/** Full combo to enter. Empty outside a combo. */
	UPROPERTY(BlueprintReadOnly)
	TArray<EActivityInputSlot> Sequence;

	/** Per-press outcome, parallel to Sequence. Everything from CurrentPressIndex on is Pending. */
	UPROPERTY(BlueprintReadOnly)
	TArray<EActivityPressResult> Results;

	/** Index into Sequence of the press being asked for. INDEX_NONE outside a combo. */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPressIndex = INDEX_NONE;

	/**
	 * How many misses the combo tolerates. Replicated so the widget can show what is left of that
	 * budget rather than a bare count nobody can read a stake into.
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxFailedPresses = 0;

	/**
	 * Bumped on every publish. Two consecutive states can otherwise be byte-identical -- the same
	 * press re-armed after a restart -- and would replicate as no change at all.
	 */
	UPROPERTY(BlueprintReadOnly)
	uint8 Revision = 0;
};
