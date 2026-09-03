// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelProgressionTypes.h"
#include "LevelSelectorActor.generated.h"

class UNiagaraComponent;
class UPointLightComponent;
class UWidgetComponent;
class UBoxComponent;
class UUserWidget;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDoorZoneOccupancyChanged, struct FLevelData, Selector, bool, bHasPlayers);

UCLASS()
class LOBBY_API ALevelSelectorActor : public AActor
{
	GENERATED_BODY()
	
public:
	ALevelSelectorActor();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDoorZoneOccupancyChanged OnDoorZoneOccupancyChanged;

	UFUNCTION(BlueprintCallable)
	void UnlockLevel();

	UFUNCTION(BlueprintImplementableEvent)
	void OnOpenDoor();

	UFUNCTION(BlueprintImplementableEvent)
	void OnCloseDoor();

	UFUNCTION(BlueprintCallable)
	FName GetLevelID() const { return LevelID; }
	UDataTable* GetLevelDataTable() const { return LevelDataTable; }

	UFUNCTION(BlueprintCallable)
	bool IsLocked() const { return LevelData.bIsLocked; }
	UFUNCTION(BlueprintCallable)
	TSoftObjectPtr<UWorld> GetLevelWorld() const { return LevelData.Level; }
	UFUNCTION(BlueprintCallable)
	FString GetLevelName() const { return LevelData.Level.IsNull() ? FString() : LevelData.Level.GetAssetName(); }
	UFUNCTION(BlueprintCallable)
	int GetLevelNumber() const { return LevelData.LevelNumber; }

	UFUNCTION(BlueprintCallable)
	void SetDoorAngle(float NewAngle);

protected:
	void BeginPlay() override;
	void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnOpenDoorTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOpenDoorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnLevelLoadTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void UpdateLevelData();

	UPROPERTY(ReplicatedUsing = OnRep_IsReplicatedLocked)
	bool bIsReplicatedLocked;

	UFUNCTION()
	void OnRep_IsReplicatedLocked();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayUnlockEffects();

	/** Shows the loading screen on every client just before the server travel is triggered. */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ShowLoadingScreen();

	/** Resolves the loading screen background image for this level from the static DataTable. */
	UTexture2D* GetLevelLoadingTexture() const;

protected:

	int32 PlayersInDoorZoneCount;

	FLevelData LevelData;

	void UpdateVisuals();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> DoorStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> Torch1NiagaraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> Torch2NiagaraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> Torch1PointLightComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> Torch2PointLightComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> LevelIndexWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> OpenDoorTriggerBoxComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> LevelLoadTriggerBoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progression")
	class UDataTable* LevelDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Progression")
	FName LevelID;

	/** Full screen widget shown while the selected level is loading. Should implement ILoadingScreenWidgetInterface. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading Screen")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	float DoorOpenAngle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MinDoorAngle = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxDoorAngle = 90.f;

};
