// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Carriable.h"
#include "ItemActor.generated.h"

class UItemAsset;
class UStaticMeshComponent;
class UNiagaraComponent;
class UAudioComponent;

UCLASS()
class ITEMS_API AItemActor : public AActor, public ICarriable
{
	GENERATED_BODY()

	AItemActor();
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;

public:

	void SetItemAsset(UItemAsset& NewItemAsset);
	UItemAsset* GetItemAsset() const { return ItemAsset; }

protected: // ICarriable

	void Pickup_Implementation(USceneComponent* AttachComponent) override;
	void Drop_Implementation() override;
	void Throw_Implementation(FVector Velocity) override;

private:

	UFUNCTION()
	void Mesh_OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	static constexpr float GroundCollisionThreshold = 0.8f;
	
	void SnapToGround();
	static constexpr float SnapToGroundMaxDistance = 1000.0f; // 10 meters 
	
private:

	// TODO: OnRep apply asset
	UPROPERTY(EditInstanceOnly)
	TObjectPtr<UItemAsset> ItemAsset;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> Audio;
};
