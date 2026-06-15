// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StationVisualProvider.h"
#include "StationVisual_StaticMesh.generated.h"

/**
 * 
 */
UCLASS()
class STATIONS_API UStationVisual_StaticMesh : public UStationVisualProvider
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent> ComponentTemplate;

	// Where to attach the holder to the mesh.
	UPROPERTY(EditAnywhere)
	FName HolderSocket;
	
	void Apply(AStationActor* Target) override;
	void Teardown(AStationActor* Target) override;
};
