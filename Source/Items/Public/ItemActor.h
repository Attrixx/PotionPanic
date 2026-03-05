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
class UHolderComponent;

UCLASS()
class ITEMS_API AItemActor : public AActor, public ICarriable
{
	GENERATED_BODY()

	AItemActor();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;

public:

	void SetItemAsset(UItemAsset& NewItemAsset);
	UItemAsset* GetItemAsset() const { return ItemAsset; }

protected: // ICarriable

	USceneComponent* GetAttachComponent_Implementation() override;
	bool TryPickup_Implementation(USceneComponent* AttachComponent) override;
	bool TryCatch_Implementation(USceneComponent* AttachComponent) override;
	bool TryTransfer_Implementation(USceneComponent* AttachComponent) override;
	void Drop_Implementation() override;
	void Throw_Implementation(FVector Velocity) override;
	
	bool TryAttachTo(USceneComponent* AttachComponent);

private:

	UFUNCTION()
	void Mesh_OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	static constexpr float GroundCollisionThreshold = 0.8f;

	bool TrySnapToGround();
	static constexpr float SnapToGroundMaxDistance = 200.0f;

	UFUNCTION()
	void OnRep_AttachComp();

	UFUNCTION()
	void ApplyItemAsset();

private:

	// Cleared after hitting something, avoid being pickup up by
	// the comp that just dropped us
	UPROPERTY(ReplicatedUsing=OnRep_AttachComp)
	TWeakObjectPtr<USceneComponent> AttachComp;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing=ApplyItemAsset)
	TObjectPtr<UItemAsset> ItemAsset;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAudioComponent> Audio;
};
