// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyPlayerPreview.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;

UCLASS()
class COREGAMEPLAY_API ALobbyPlayerPreview : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ALobbyPlayerPreview();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	
	virtual void BeginPlay() override;

public:

	UFUNCTION(Server, Reliable)
	void SetPlayerColor(FColor color);

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* PlayerMesh;

protected:

	UPROPERTY(ReplicatedUsing = OnRep_PreviewColor)
	FColor PreviewColor;

	UFUNCTION()
	void OnRep_PreviewColor();

};
