// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnableObject.generated.h"

UCLASS()
class POTIONPANIC_API ASpawnableObject : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ASpawnableObject();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent* SpawnerMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootSceneComponent;

public:	
	
	virtual void Tick(float DeltaTime) override;

};
