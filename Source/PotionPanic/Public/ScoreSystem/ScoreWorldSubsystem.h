#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ScoreWorldSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, int32, NewScore);

UCLASS()
class POTIONPANIC_API UScoreWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void AddScore(int32 Points);

    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void SetScore(int32 NewScore);

    UFUNCTION(BlueprintCallable, Category = "Scoring")
    void ResetScore();

    UFUNCTION(BlueprintPure, Category = "Scoring")
    int32 GetScore() const { return TotalScore; }

    UPROPERTY(BlueprintAssignable, Category = "Scoring")
    FOnScoreChanged OnScoreChanged;

private:
    UPROPERTY(VisibleAnywhere, Category = "Scoring")
    int32 TotalScore = 0;

    void BroadcastScoreChanged();
};
