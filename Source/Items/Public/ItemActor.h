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
	
public:
	AItemActor();
	void OnConstruction(const FTransform& Transform) override;
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void SetItemAsset(UItemAsset* NewItemAsset);

	UFUNCTION(BlueprintCallable, Category = "Item")
	virtual void DestroyItem(bool bPlayFeedback = true);

	UFUNCTION(BlueprintPure, Category = "Item")
	UItemAsset* GetItemAsset() const { return ItemAsset; }

	UFUNCTION(BlueprintPure, Category = "Item")
	bool IsBreakable() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	float GetBreakImpulseThreshold() const;

	UFUNCTION(BlueprintPure, Category = "Item")
	TArray<FName> GetTransformationFlags() const;

private:
	void ApplyVisualsFromAsset(UItemAsset* NewItemAsset);
	void PlaySpawnFeedback() const;
	void PlayDestroyFeedback() const;

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
