// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "RecipeSystem/Recipe.h"
#include "StationActor.generated.h"

class USocketComponent;
class UStationComponent;
class USpawnerComponent;
class UCamTargetComponent;
class UWidgetComponent;
struct FInputItemGroup;
struct FAbilityEndedData;
struct FGameplayTag;
class UGameplayAbility;
class USocketableComponent;

UCLASS()
class POTIONPANIC_API AStationActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	AStationActor();

protected:
	void BeginPlay() override;

	UAbilitySystemComponent* GetAbilitySystemComponent() const override
	{
		return AbilitySystemComponent;
	}

	void GiveStartupAbilities();
	FRecipe* FindMatchingRecipe(const FInputItemGroup& Items) const;
	void OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);
	void OnHeldChanged(USocketableComponent* OldHeld, USocketableComponent* NewHeld);
	void Store();

public:

	void StartProcessing(APawn* ProcessInstigator, FInputItemGroup& Items);
	FRecipe* GetCurrentRecipe() const { return CurrentRecipe; }
	APawn* GetCurrentProcessInstigator() const { return CurrentProcessInstigator; }
	bool CanAcceptItem(TSubclassOf<AActor> ItemClass) const;

	void ShowInteractionUI(bool bShow);
	void ShowAnimatedProgress(float Duration, bool bAutoHide);
	void UpdateProgressUI(float Progress);
	void HideProgressUI();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowInteractionUI(bool bShow);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowAnimatedProgress(float Duration, bool bAutoHide);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HideProgressUI();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USocketComponent> SocketComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStationComponent> StationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USpawnerComponent> SpawnerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCamTargetComponent> CamTargetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditAnywhere)
	FGameplayTag StationTag;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> RecipesDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

private:

	FRecipe* CurrentRecipe;
	APawn* CurrentProcessInstigator;
	bool bDestroySpawnedItem = true;
	bool bUsedSocketItem = false;
	AActor* SpawnedItem = nullptr;

};
