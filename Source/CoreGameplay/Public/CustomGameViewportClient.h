// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "CustomGameViewportClient.generated.h"

DEFINE_LOG_CATEGORY_STATIC(MS_CustomGameViewportClient, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalPlayerJoinRequest, int32, ControllerId);

/**
 * 
 */
class UInputMappingContext;
class UInputAction;

/**
 * 
 */
UCLASS(Config=Game)
class COREGAMEPLAY_API UCustomGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:

	bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	FOnLocalPlayerJoinRequest OnLocalPlayerJoinRequest;

protected:

	UPROPERTY(Config, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> JoinMappingContext;

	UPROPERTY(Config, EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputAction> JoinAction;
	
};
