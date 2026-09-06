// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepPresentation.h"
#include "ActivityStepResult.h"
#include "ActivityStepSettings.h"
#include "InputComboActivityStep.generated.h"

enum class EActivityInputSlot : uint8;

UENUM(BlueprintType)
enum class EActivityPressResult : uint8
{
	Pending = 0,
	Hit,
	Miss,
};

USTRUCT(BlueprintType)
struct FInputComboActivityPresentation : public FActivityStepPresentationCustomInfo
{
	GENERATED_BODY()
	
	/** Full combo to enter. */
	UPROPERTY(BlueprintReadOnly)
	TArray<EActivityInputSlot> Sequence;

	/** Per-press outcome, parallel to Sequence. */
	UPROPERTY(BlueprintReadOnly)
	TArray<EActivityPressResult> Results;

	/** Index into Sequence of the press being asked for. */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPressIndex = INDEX_NONE;

	/** How many misses the combo tolerates. */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxFailedPresses = 0;
};

UCLASS(DisplayName = "Input Combo")
class ACTIVITIES_API UInputComboActivitySettings : public UActivityStepSettings
{
	GENERATED_BODY()

#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

public:

	/**
	 * Which of the four inputs the combo may draw from. At least one is required.
	 * @note uint8 rather than the enum: the checkbox list only shows up on a plain integer
	 *       property, an enum-typed one falls through to a single-select dropdown.
	 */
	UPROPERTY(EditAnywhere, Category = "", meta = (Bitmask, BitmaskEnum = "EActivityInputSlot"))
	uint8 AllowedSlots = 0;

	/** How many presses the combo asks for. */
	UPROPERTY(EditAnywhere, Category = "", meta = (ClampMin = 1))
	int32 PressCount = 10;

	/**
	 * How many presses may be missed and still pass. A miss is either the wrong input or none at
	 * all before the press times out; going over ends the step immediately, since no amount of
	 * remaining presses can bring it back.
	 */
	UPROPERTY(EditAnywhere, Category = "", meta = (ClampMin = 0))
	int32 MaxFailedPresses = 1;

	/** How long each press may be waited for before it counts as missed. */
	UPROPERTY(EditAnywhere, Category = "", meta = (ClampMin = 0.01))
	float PressTimeoutSeconds = 1.f;

	/** Whether the same input may be drawn twice in a row. Ignored when only one is allowed. */
	UPROPERTY(EditAnywhere, Category = "")
	bool bAllowConsecutiveRepeats = true;
};

/**
 * Asks one player for a sequence of presses among four inputs, one at a time and each on a clock.
 *
 * The combo runs on the authority, which draws the sequence, holds the timers and decides every
 * press. The instigator's client only says which input it pressed; the widget everyone sees comes
 * from the replicated presentation, so the table watches the combo unfold without being able to
 * take part in it.
 *
 * There is no attempt at validating that a press was physically possible: an instigator that lies
 * about its presses gets a better score, and that is a trade we are making knowingly.
 */
UCLASS()
class ACTIVITIES_API UInputComboActivityStep : public UActivityStep
{
	GENERATED_BODY()

	void StartStep_Implementation(AActor* LastInstigator) override;
	void OnInteract_Implementation(AActor* Instigator) override;
	void CancelStep_Implementation() override;
	void OnActivityInput_Implementation(AActor* Instigator, EActivityInputSlot Slot) override;
	void OnCancelRequested_Implementation(AActor* Instigator) override;

	/** Draws the sequence, takes the instigator's inputs over and asks for the first press. */
	void BeginCombo(AActor* Instigator);

	/** @return A slot drawn from AllowedSlots, avoiding Previous when repeats are disallowed. */
	EActivityInputSlot DrawSlot(EActivityInputSlot Previous) const;

	/** Records the current press and moves on, ending the step when it can be decided. */
	void RecordPress(EActivityPressResult Result);

	/** Ends the step, giving the inputs back and taking the widget down first. */
	void Finish(EActivityStepStatus Status);

	/** Releases the captured instigator and clears the presentation. Safe to call twice. */
	void ReleaseCapture();

	void ArmPressTimeout();
	void ClearPressTimeout();
	void PublishPresentation();

	friend UInputComboActivitySettings;

	uint8 AllowedSlots = 0;
	int32 PressCount = 10;
	int32 MaxFailedPresses = 1;
	float PressTimeoutSeconds = 1.f;
	bool bAllowConsecutiveRepeats = true;

	TArray<EActivityInputSlot> Sequence;
	TArray<EActivityPressResult> Results;
	TWeakObjectPtr<AActor> CapturedInstigator;
	int32 CurrentPressIndex = INDEX_NONE;
	int32 SuccessCount = 0;
	int32 FailureCount = 0;
	bool bWaitingForInstigator = false;
	FTimerHandle PressTimeoutHandle;
	
public: // Widget Helpers
	
	UFUNCTION(BlueprintPure)
	static EActivityInputSlot GetCurrentSlot(const FInputComboActivityPresentation& Data);
	
	UFUNCTION(BlueprintPure)
	static float GetComboProgressAlpha(const FInputComboActivityPresentation& Data);

	UFUNCTION(BlueprintPure)
	static float GetComboErrorAlpha(const FInputComboActivityPresentation& Data);
};
