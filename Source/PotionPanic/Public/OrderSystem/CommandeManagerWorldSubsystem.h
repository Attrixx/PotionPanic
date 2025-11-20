#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "OrderSystem/OrderClient.h"
#include "CommandeManagerWorldSubsystem.generated.h"

UENUM(BlueprintType)
enum class EOrderRoundResult : uint8
{
    PerfectWin UMETA(DisplayName = "Win 2/2"),
    PartialWin UMETA(DisplayName = "Win 1/2"),
    Lose       UMETA(DisplayName = "Lose 0/2"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FOnRoundEnded,
    AOrderClient*, Client,
    EOrderRoundResult, Result,
    int32, SuccessCount
);

UCLASS()
class POTIONPANIC_API UCommandeManagerWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "Orders|Events")
    FOnRoundEnded OnRoundEnded;

    UFUNCTION(BlueprintCallable, Category = "Orders")
    void RegisterClient(AOrderClient* Client);

    UFUNCTION(BlueprintCallable, Category = "Orders")
    void StartRound(AOrderClient* Client);

    UFUNCTION(BlueprintCallable, Category = "Orders")
    bool ValidateDishForClient(AOrderClient* Client, AActor* DishActor) const;

private:
    struct FRoundState
    {
        int32 Served = 0;
        int32 Success = 0;
        bool bInRound = false;
        FTimerHandle NextOrderHandle;
        FTimerHandle RestartHandle;
        TArray<int32> Pool;
        int32 LastIndex = INDEX_NONE;
    };

    TMap<TWeakObjectPtr<AOrderClient>, FRoundState> States;
    TArray<FClientOrderEntry> CachedOrders;

    int32 OrdersPerRound = 2;
    float NextOrderDelay = 2.f;
    float RoundRestartDelay = 3.f;
    bool bAutoStartOnRegister = true;
    bool bAvoidSameOrderTwice = true;
    bool bReloadLevelOnRoundEnd = false;

    UFUNCTION()
    void HandleOrderFinished(AOrderClient* Client, const FClientOrderEntry& Order, bool bSuccess);

    void IssueNextOrder(AOrderClient* Client);
    void ScheduleNextOrder(AOrderClient* Client);
    void EndRound(AOrderClient* Client);
    void RefillAndShufflePool(FRoundState& State);
    int32 DrawFromPool(FRoundState& State);
    void ReloadCurrentLevel();
};
