#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OrderSystem/OrderClient.h"
#include "OrderHUDWidget.generated.h"

class UImage;
class UTextBlock;

/**
 * Simple HUD widget that mirrors the current client order in UI with an icon, name and remaining time.
 */
UCLASS()
class POTIONPANIC_API UOrderHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    UFUNCTION(BlueprintCallable, Category = "Orders")
    void UpdateOrder(const FClientOrderEntry& Order, float RemainingTime, bool bIsActive);

    UFUNCTION(BlueprintCallable, Category = "Orders")
    void ShowResult(bool bSuccess);

protected:
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> Image_OrderIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_OrderName;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Timer;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> Text_Status;

    UPROPERTY(EditDefaultsOnly, Category = "Orders")
    FText SuccessText = NSLOCTEXT("OrderUI", "OrderSuccess", "Commande validee !");

    UPROPERTY(EditDefaultsOnly, Category = "Orders")
    FText FailureText = NSLOCTEXT("OrderUI", "OrderFailed", "Commande ratee");

    UPROPERTY(EditDefaultsOnly, Category = "Orders")
    FText UnknownOrderText = NSLOCTEXT("OrderUI", "OrderUnknown", "Commande inconnue");

private:
    void EnsureFallbackWidgets();
    void UpdateOrderName(const FClientOrderEntry& Order);
    void UpdateIcon(const FClientOrderEntry& Order);
    void UpdateTimer(float RemainingTime);
    void ClearStatus();
    FText ResolveOrderName(const FClientOrderEntry& Order) const;
    UTexture2D* ResolveIcon(const FClientOrderEntry& Order);
};
