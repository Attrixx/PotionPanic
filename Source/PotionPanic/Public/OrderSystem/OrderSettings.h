#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "OrderSystem/OrdersDataAsset.h"
#include "OrderSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Orders Settings"))
class POTIONPANIC_API UOrderSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Config, Category = "Orders")
    TSoftObjectPtr<UOrdersDataAsset> OrdersAsset;

    UPROPERTY(EditAnywhere, Config, Category = "Round")
    int32 OrdersPerRound = 2;

    UPROPERTY(EditAnywhere, Config, Category = "Round")
    float NextOrderDelay = 2.f;

    UPROPERTY(EditAnywhere, Config, Category = "Round")
    float RoundRestartDelay = 3.f;

    UPROPERTY(EditAnywhere, Config, Category = "Round")
    bool bAutoStartOnRegister = true;

    UPROPERTY(EditAnywhere, Config, Category = "Round")
    bool bAvoidSameOrderTwice = true;

    UPROPERTY(EditAnywhere, Config, Category = "Round")
    bool bReloadLevelOnRoundEnd = false;
};
