#pragma once

#include "CoreMinimal.h"
#include "DistinguishComponent.h"
#include "WidgetDistinguishComponent.generated.h"

class UUserWidget;

UCLASS(Blueprintable)
class POTIONPANIC_API UWidgetDistinguishComponent : public UDistinguishComponent
{
	GENERATED_BODY()

public:
	UWidgetDistinguishComponent();

protected:
	void BeginPlay() override;

	void OnActivate() override;
	void OnDeactivate() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Distinguish")
	TSubclassOf<UUserWidget> UserWidget = nullptr;
};