// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class UItemAsset;
class UCarriableComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UAudioComponent;

UCLASS()
class ITEMS_API AItemActor : public AActor
{
	GENERATED_BODY()
	
	AItemActor();
	void OnConstruction(const FTransform& Transform) override;

public:

	void SetItemAsset(UItemAsset& NewItemAsset);
	UItemAsset* GetItemAsset() const { return ItemAsset; }

private:

	UPROPERTY(EditInstanceOnly)
	TObjectPtr<UItemAsset> ItemAsset;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UCarriableComponent> Carriable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> Audio;
};
