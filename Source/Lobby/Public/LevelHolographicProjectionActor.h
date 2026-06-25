// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelProgressionTypes.h"
#include "LevelHolographicProjectionActor.generated.h"

class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelDataChanged, const FLevelData&, LevelData);

UCLASS()
class LOBBY_API ALevelHolographicProjectionActor : public AActor
{
	GENERATED_BODY()
	
public:
	ALevelHolographicProjectionActor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLevelDataChanged OnLevelDataChanged;

	UFUNCTION(BlueprintImplementableEvent)
	void Show();
	UFUNCTION(BlueprintImplementableEvent)
	void Hide();

	void SetIsShowing(bool bNewShowing);
	void SetLevelData(const FLevelData& NewData);

protected:
	UFUNCTION()
	void OnRep_IsShowing();

	UFUNCTION()
	void OnRep_LevelData();

	UPROPERTY(ReplicatedUsing = OnRep_IsShowing)
	bool bIsShowing;

	UPROPERTY(ReplicatedUsing = OnRep_LevelData)
	FLevelData CurrentLevelData;

public:
	UFUNCTION(BlueprintCallable, Category = "Level Progression")
	FLevelData GetCurrentLevelData() const { return CurrentLevelData; }

	UFUNCTION(BlueprintCallable)
	void SetProgress(float InProgress);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRootComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HolographicProjectionWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialParameterCollection> HolographicProjectionMaterialParameterCollection;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName OpacityParameterName = "Opacity";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName RaysLengthParameterName = "RaysLength";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxRayLength = 1000.f;

};
