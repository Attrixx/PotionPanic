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

    FClientOrderEntry()
        : OrderId(NAME_None), DisplayText(FText::GetEmpty()), Payload(nullptr)
    {
    }
};

UCLASS()
class POTIONPANIC_API AOrderClient : public AActor
{
    GENERATED_BODY()

public:
    AOrderClient();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Order")
    void TryServeDish(AActor *DishActor);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent *OrderTextComp;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    TArray<FClientOrderEntry> PossibleOrders;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
    float OrderDuration;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Order")
    FClientOrderEntry CurrentOrder;

    bool bHasActiveOrder;
    float RemainingTime;
    FTimerHandle OrderTimerHandle;

    void StartOrder();
    void OrderTimerTick();
    void CancelOrder();
    void CompleteOrder(AActor *DishActor);

    void UpdateText3D(const FText &NewText);
    bool DoesDishMatchCurrentOrder(AActor *DishActor) const;
};
