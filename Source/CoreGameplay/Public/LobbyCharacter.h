// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "LobbyCharacter.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UInputAction;

UCLASS()
class COREGAMEPLAY_API ALobbyCharacter : public ACharacter
{
	GENERATED_BODY()
	
public:	
	
	ALobbyCharacter();

	UFUNCTION()
	void SetPlayerColor(FColor Color);

protected:
	
	void PossessedBy(AController* NewController) override;
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void OnRep_PlayerState() override;
	void OnPlayerStateReady();

protected:

	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMeshComponent* PlayerMesh;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(ReplicatedUsing=OnRep_CharacterColor)
	FColor CharacterColor = FColor::White;

	UFUNCTION()
	void OnRep_CharacterColor();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
