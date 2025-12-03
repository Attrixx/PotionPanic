// Copyright 2025

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndMenuWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReplayRequested);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReturnToMenuRequested);

UCLASS()
class POTIONPANIC_API UEndMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void NativeConstruct() override;
    void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "EndMenu")
    void SetEndState(bool bIsVictory, int32 Score);

    UPROPERTY(BlueprintAssignable, Category = "EndMenu|Event")
    FOnReplayRequested OnReplayRequested;

    UPROPERTY(BlueprintAssignable, Category = "EndMenu|Event")
    FOnReturnToMenuRequested OnReturnToMenuRequested;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Result;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_Score;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_Replay;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> Button_MainMenu;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_ReplayLabel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_MainMenuLabel;

    UPROPERTY(EditAnywhere, Category = "EndMenu")
    FText VictoryText = FText::FromString(TEXT("Victoire !"));

    UPROPERTY(EditAnywhere, Category = "EndMenu")
    FText DefeatText = FText::FromString(TEXT("Defaite..."));

    UPROPERTY(EditAnywhere, Category = "EndMenu")
    FText ReplayButtonText = FText::FromString(TEXT("Rejouer"));

    UPROPERTY(EditAnywhere, Category = "EndMenu")
    FText MainMenuButtonText = FText::FromString(TEXT("Menu principal"));

    UFUNCTION()
    void HandleReplayClicked();

    UFUNCTION()
    void HandleMainMenuClicked();
};
