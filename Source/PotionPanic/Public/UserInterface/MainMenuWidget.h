// Copyright 2025

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayRequested);

UCLASS()
class POTIONPANIC_API UMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void NativeConstruct() override;
    void NativeDestruct() override;

    UPROPERTY(BlueprintAssignable, Category = "MainMenu|Event")
    FOnPlayRequested OnPlayRequested;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Play;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_PlayLabel;

    UPROPERTY(EditAnywhere, Category = "MainMenu")
    FText PlayButtonText = FText::FromString(TEXT("Jouer"));

    UFUNCTION()
    void HandlePlayClicked();
};
