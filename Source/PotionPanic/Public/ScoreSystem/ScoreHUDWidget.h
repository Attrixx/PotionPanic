#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScoreHUDWidget.generated.h"

class UTextBlock;
class UScoreWorldSubsystem;

UCLASS()
class POTIONPANIC_API UScoreHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock *ScoreText = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    FText ScorePrefix = FText::FromString(TEXT("Score: "));

    UPROPERTY(Transient)
    UScoreWorldSubsystem *ScoreSubsystem = nullptr;

    UFUNCTION()
    void HandleScoreChanged(int32 NewScore);

    void UpdateScoreText(int32 NewScore);
};
