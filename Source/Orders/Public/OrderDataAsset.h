#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "OrderDataAsset.generated.h"

UCLASS(BlueprintType)
class ORDERS_API UOrderDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Order|Validation")
	bool IsOrderDefinitionValid(FText& OutFailureReason) const;

	UFUNCTION(BlueprintCallable, Category = "Order|Matching")
	bool MatchesDeliveredItem(const FPrimaryAssetId& DeliveredItemId, FText& OutFailureReason) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FText OrderName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	FPrimaryAssetId RequiredOutputItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	TArray<FPrimaryAssetId> AdditionalAcceptedOutputItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	TArray<FName> RequiredOutputDataTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order")
	bool bRequireAllOutputDataTags = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Scoring")
	int32 BaseScore = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Timing", meta = (ClampMin = "0.1"))
	float TimeLimitSeconds = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Scoring", meta = (ClampMin = "0"))
	int32 TimeBonusMax = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Scoring", meta = (ClampMin = "0"))
	int32 WrongDeliveryPenalty = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Scoring", meta = (ClampMin = "0"))
	int32 ExpirePenalty = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Order|Priority")
	int32 Priority = 0;
};
