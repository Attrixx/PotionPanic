// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerAdded, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerRemoved, APlayerState*, PlayerState);

class ALevelSequenceActor;

UENUM(BlueprintType)
enum class ETriggerBoxType : uint8
{
	LobbyInterface,
	Entrance,
	Interior
};

UENUM()
enum class ECameraPosition : uint8
{
	Exterior,
	LobbyInterface,
	Entrance,
	Interior
};

UENUM(BlueprintType)
enum class ELevelSequenceType : uint8
{
	ExteriorToLobbyInterface,
	ExteriorToEntrance,
	ExteriorToInterior,
	EntranceToLobbyInterface,
	EntranceToInterior,
	InteriorToLobbyInterface,
	OpenDoors
};

USTRUCT(BlueprintType)
struct FLevelSequenceInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELevelSequenceType SequenceType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ALevelSequenceActor> SequenceActor;
};

UCLASS()
class COREGAMEPLAY_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRegisterLevelSequences(const TArray<FLevelSequenceInfo>& LevelSequences);

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool AreAllPlayersReady() const;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayLevelSequence(ECameraPosition TargetCameraPosition);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOpenDoors(bool bOpen = true);

	ALevelSequenceActor* GetLevelSequenceActor(ELevelSequenceType SequenceType) const;

protected:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

public:

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerAdded OnPlayerAdded;

	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerRemoved OnPlayerRemoved;

private:

	TArray<FColor> DefaultColors = {
		FColor::Yellow,
		FColor::Red,
		FColor::Green,
		FColor::Blue
	};
	TArray<bool> AvailableColors = { true, true, true, true };

	UPROPERTY(Replicated)
	TArray<FLevelSequenceInfo> RegisteredLevelSequences;

};
