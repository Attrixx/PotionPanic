// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

#include "LobbyPlayerPreview.generated.h"

UCLASS()
class COREGAMEPLAY_API ALobbyPlayerPreview : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ALobbyPlayerPreview();

protected:
	
	virtual void BeginPlay() override;

public:	
	
	virtual void Tick(float DeltaTime) override;
	void SetPlayerColor(FColor color);

public:
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly)
	UStaticMeshComponent* PlayerMesh;

};
