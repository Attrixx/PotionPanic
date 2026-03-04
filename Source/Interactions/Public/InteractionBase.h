// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "InteractionBase.generated.h"

UENUM(BlueprintType)
enum class EInteractionResult : uint8
{
	Default,
	// CriticalSuccess, // TODO Maybe ?
	Success,
	Fail,
	CriticalFail
};

USTRUCT(BlueprintType)
struct INTERACTIONS_API FInteractionOutput
{
	GENERATED_BODY()

	EInteractionResult InteractionResult = EInteractionResult::Default;
	uint32 Score = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractionOutputDelegate, FInteractionOutput, InteractionOutput);

USTRUCT(BlueprintType)
struct INTERACTIONS_API FInteractionContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	AController* Instigator;
	FInteractionOutputDelegate OnInteractionFinished;
};

class UInteractionSetting;

UCLASS(Abstract, BlueprintType)
class INTERACTIONS_API UInteractionBase : public UObject
{
	GENERATED_BODY()
	
public:	
	static UInteractionBase* CreateInteraction(UObject* Outer, UInteractionSetting* Settings);
	
	UFUNCTION(BlueprintCallable)
	virtual void Init(UInteractionSetting* Settings) PURE_VIRTUAL(UInteractionBase::Init, );
	
	UFUNCTION(BlueprintCallable)
	virtual void StartInteraction(const FInteractionContext& Context) PURE_VIRTUAL(UInteractionBase::StartInteraction, );
};
