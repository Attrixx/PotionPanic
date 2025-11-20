#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OrderClient.generated.h"

class UTextRenderComponent;

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

    FClientOrderEntry()
        : OrderId(NAME_None), DisplayText(FText::GetEmpty()), Payload(nullptr), Duration(30.f)
    {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOrderStarted, const FClientOrderEntry&, Order);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnOrderFinished, AOrderClient*, Client, const FClientOrderEntry&, Order, bool, bSuccess);

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

    UFUNCTION(BlueprintPure, Category = "Order")
    bool HasActiveOrder() const { return bHasActiveOrder; }

    UFUNCTION(BlueprintPure, Category = "Order")
    const FClientOrderEntry& GetCurrentOrder() const { return CurrentOrder; }

    UFUNCTION(BlueprintPure, Category = "Order")
    bool CheckDishMatchesCurrentOrder(AActor* DishActor) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* OrderTextComp;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Order")
    FClientOrderEntry CurrentOrder;

    bool bHasActiveOrder = false;
    float RemainingTime = 0.f;

    FTimerHandle OrderTimerHandle;

    void OrderTimerTick();
    void CancelOrder();
    void CompleteOrder(AActor* DishActor);

    void UpdateText3D(const FText& NewText);
};
