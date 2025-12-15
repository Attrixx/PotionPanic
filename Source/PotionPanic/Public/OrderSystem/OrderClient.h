#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPath.h"
#include "OrderClient.generated.h"

class UTextRenderComponent;
class UTexture2D;

USTRUCT(BlueprintType)
struct FClientOrderEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    FName OrderId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    FText DisplayText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    TObjectPtr<UObject> Payload;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    float Duration = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    TSoftObjectPtr<UTexture2D> Icon;

    FClientOrderEntry()
        : OrderId(NAME_None)
        , DisplayText(FText::GetEmpty())
        , Payload(nullptr)
        , Duration(30.f)
        , Icon(nullptr)
    {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderStarted, const FClientOrderEntry&, Order);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnOrderFinished, AOrderClient*, Client, const FClientOrderEntry&, Order, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnOrderUpdated, const FClientOrderEntry&, Order, float, RemainingTime, bool, bIsActive);

UCLASS()
class POTIONPANIC_API AOrderClient : public AActor
{
    GENERATED_BODY()

public:
    AOrderClient();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Order")
    void BeginOrder(const FClientOrderEntry& NewOrder);

    UFUNCTION(BlueprintCallable, Category = "Order")
    void TryServeDish(AActor* DishActor);

    UPROPERTY(BlueprintAssignable, Category = "Order|Events")
    FOnOrderStarted OnOrderStarted;

    UPROPERTY(BlueprintAssignable, Category = "Order|Events")
    FOnOrderFinished OnOrderFinished;

    UPROPERTY(BlueprintAssignable, Category = "Order|Events")
    FOnOrderUpdated OnOrderUpdated;

    UFUNCTION(BlueprintPure, Category = "Order")
    bool HasActiveOrder() const { return bHasActiveOrder; }

    UFUNCTION(BlueprintPure, Category = "Order")
    const FClientOrderEntry& GetCurrentOrder() const { return CurrentOrder; }

    UFUNCTION(BlueprintPure, Category = "Order")
    float GetRemainingTime() const { return RemainingTime; }

    UFUNCTION(BlueprintPure, Category = "Order")
    bool CheckDishMatchesCurrentOrder(AActor* DishActor) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    bool bUseWorldText = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* OrderTextComp;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_OrderState, Category = "Order")
    FClientOrderEntry CurrentOrder;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_OrderState, Category = "Order")
    float RemainingTime = 0.f;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_OrderState, Category = "Order")
    bool bHasActiveOrder = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Replicated, Category = "Order")
    bool bLastOrderSuccess = false;

    // Local cache used to detect state transitions on clients when replicated data changes.
    bool bCachedHadActiveOrder = false;

    FTimerHandle OrderTimerHandle;

    void OrderTimerTick();
    void CancelOrder();
    void CompleteOrder(AActor* DishActor);

    void UpdateText3D(const FText& NewText);
    void BroadcastOrderUpdated();

    UFUNCTION()
    void OnRep_OrderState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
