#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/QTETypes.h"
#include "QTEWidgetBase.generated.h"

class UQTEComponent;

UCLASS(Abstract, Blueprintable)
class QTE_API UQTEWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "QTE")
	void BindToQTEComponent(UQTEComponent* InQTEComponent);

	UFUNCTION(BlueprintPure, Category = "QTE")
	UQTEComponent* GetBoundQTEComponent() const;

protected:
	void NativeDestruct() override;

	virtual void OnBoundQTEComponentChanged(UQTEComponent* PreviousQTEComponent, UQTEComponent* NewQTEComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "QTE")
	void HandleQTEStarted(const FQTERuntimeState& State);

	UFUNCTION(BlueprintNativeEvent, Category = "QTE")
	void HandleQTEStepChanged(const FQTERuntimeState& State);

	UFUNCTION(BlueprintNativeEvent, Category = "QTE")
	void HandleQTEUpdated(const FQTERuntimeState& State);

	UFUNCTION(BlueprintNativeEvent, Category = "QTE")
	void HandleQTEFinished(const FQTEResult& Result);

private:
	void UnbindFromCurrentComponent();

private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "QTE", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UQTEComponent> BoundQTEComponent = nullptr;
};
