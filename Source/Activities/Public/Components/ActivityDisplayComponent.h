// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "ActivityStepPresentation.h"
#include "ActivityDisplayComponent.generated.h"

class UActivityExecutor;
class UActivityStep;

/**
 * Draws whatever the activity running on this actor wants displayed, for everyone who can see it.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class ACTIVITIES_API UActivityDisplayComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UActivityDisplayComponent();

protected:

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	/** Lays the widget's plane parallel to the local camera's view plane. */
	void AlignToLocalCamera();

	UFUNCTION()
	void Executor_OnStepPresentationChanged(UActivityExecutor* InExecutor);

	/** Swaps the widget class when the step changed, then pushes the presentation into it. */
	void ApplyPresentation(const FActivityStepPresentation& Presentation);

	UPROPERTY(Transient)
	TObjectPtr<UActivityExecutor> Executor;

	/** Step class the current widget was built for. Null when nothing is displayed. */
	UPROPERTY(Transient)
	TSubclassOf<UActivityStep> CurrentStepClass;
};
