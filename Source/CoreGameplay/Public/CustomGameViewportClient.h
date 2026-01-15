// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "CustomGameViewportClient.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalPlayerJoinRequest, int32, ControllerId);

/**
 * 
 */
UCLASS()
class COREGAMEPLAY_API UCustomGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:

	bool InputKey(const FInputKeyEventArgs& EventArgs) override;

	FOnLocalPlayerJoinRequest OnLocalPlayerJoinRequest;
	
};
