#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OrderSystem/OrderClient.h"
#include "OrdersDataAsset.generated.h"

UCLASS(BlueprintType)
class POTIONPANIC_API UOrdersDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FClientOrderEntry> Orders;
};
