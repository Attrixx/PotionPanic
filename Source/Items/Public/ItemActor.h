// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

struct FItemRow;
class USocketableComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UAudioComponent;

UCLASS()
class ITEMS_API AItemActor : public AActor
{
	GENERATED_BODY()
	
	AItemActor();

public:

	void SetItem(const FItemRow& NewItem);
	const FItemRow* GetItem() const { return Item; }

private:

	const FItemRow* Item;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USocketableComponent> SocketableRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> Audio;
};
