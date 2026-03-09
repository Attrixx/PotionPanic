// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Carriable.h"
#include "GameplayTagContainer.h"
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
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable)
	void SetItemAsset(UItemAsset* NewItemAsset);

	UFUNCTION(BlueprintCallable)
	const FGameplayTagContainer& GetItemTags() const { return ItemTags; }
	
	UFUNCTION(BlueprintCallable)
	void AppendItemTags(const FGameplayTagContainer& NewItemTags);
	
	UFUNCTION(BlueprintCallable)
	void RemoveItemTag(const FGameplayTagContainer& ItemTagsToRemove);

protected: // ICarriable

	UPrimitiveComponent* GetPrimitive_Implementation() const override;
	FName GetStandaloneCollisionProfileName_Implementation() const override;
	FName GetCarriedCollisionProfileName_Implementation() const override;

private:

	UFUNCTION()
	void Mesh_OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	static constexpr float GroundCollisionThreshold = 0.8f;

	UFUNCTION()
	void OnRep_ItemAsset();

	void ApplyItemAsset();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UNiagaraComponent> Niagara;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UAudioComponent> Audio;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carriable")
	FName StandaloneCollisionProfileName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carriable")
	FName CarriedCollisionProfileName;

private:

	UPROPERTY(EditInstanceOnly, ReplicatedUsing=OnRep_ItemAsset)
	TObjectPtr<UItemAsset> ItemAsset;

	FGameplayTagContainer ItemTags;
};
