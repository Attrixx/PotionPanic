#pragma once

#include "CoreMinimal.h"
#include "ActivityStep.h"
#include "ActivityStepResult.h"
#include "ActivityStepSettings.h"
#include "Core/QTETypes.h"
#include "QTEActivityStep.generated.h"

class UQTEDefinitionDataAsset;
class UQTEComponent;
class UQTEWidgetBase;
class FDataValidationContext;
enum class EDataValidationResult : uint8;

UCLASS()
class QTE_API UQTEActivityStepSetting : public UActivityStepSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

protected:
	UActivityStep* CreateStep_Implementation(UObject* Outer) const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|Recipe")
	TObjectPtr<UQTEDefinitionDataAsset> QTEDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QTE|UI")
	TSubclassOf<UQTEWidgetBase> QTEWidgetClass;
};

UCLASS()
class QTE_API UQTEActivityStep : public UActivityStep
{
	GENERATED_BODY()

public:
	void StartStep_Implementation(AActor* LastInstigator) override;
	void OnInteract_Implementation(AActor* Instigator) override;
	void CancelStep_Implementation() override;

private:
	bool TryStartQTE(AActor* Instigator);
	void HandleAuthorityQTEFinished(int32 RequestId, const FQTEAuthorityResult& AuthorityResult);
	void FinishFromResult(const FQTEAuthorityResult& AuthorityResult);
	EActivityStepStatus ResolveStepStatus(EQTEState FinalState) const;
	void ShowQTEActivityDisplay(AActor* Instigator);
	void HideQTEActivityDisplay();
	void UnbindAuthorityDelegate();

private:
	friend UQTEActivityStepSetting;

	UPROPERTY()
	TObjectPtr<UQTEDefinitionDataAsset> QTEDefinition = nullptr;

	UPROPERTY()
	TSubclassOf<UQTEWidgetBase> QTEWidgetClass;

	TWeakObjectPtr<UQTEComponent> ActiveQTEComponent;
	TWeakObjectPtr<AActor> ActiveDisplayActor;
	int32 ActiveRequestId = INDEX_NONE;
	bool bWaitingForInstigator = false;
	FActivityStepResult StepResult;
};
