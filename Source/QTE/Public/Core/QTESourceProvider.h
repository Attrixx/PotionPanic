#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "QTESourceProvider.generated.h"

UINTERFACE(BlueprintType)
class QTE_API UQTESourceProvider : public UInterface
{
	GENERATED_BODY()
};

class QTE_API IQTESourceProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "QTE")
	UObject* GetQTESourceObject() const;
};
