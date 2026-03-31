#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QTEActivityDisplay.generated.h"

class UQTEComponent;
class UQTEWidgetBase;

UINTERFACE(BlueprintType)
class QTE_API UQTEActivityDisplay : public UInterface
{
	GENERATED_BODY()
};

class QTE_API IQTEActivityDisplay
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QTE")
	void ShowQTEActivityStep(UQTEComponent* InQTEComponent, TSubclassOf<UQTEWidgetBase> InWidgetClass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QTE")
	void HideQTEActivityStep();
};
