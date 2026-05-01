// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/DataTable.h"
#include "LobbyGameState.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerAdded, APlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerRemoved, APlayerState*, PlayerState);

class ALevelSequenceActor;

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
	
	// Optional, useful only to manually identify a sequence (ex: OpenDoors)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELevelSequenceType SequenceType = ELevelSequenceType::OpenDoors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraPosition FromArea = ECameraPosition::Exterior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECameraPosition ToArea = ECameraPosition::Exterior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ALevelSequenceActor> SequenceActor{};
};

UCLASS()
class LOBBY_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void ServerRegisterLevelSequences(const TArray<FLevelSequenceInfo>& LevelSequences);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lobby")
	const TArray<FLevelSequenceInfo>& GetRegisteredLevelSequences() const { return RegisteredLevelSequences; }

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool AreAllPlayersReady() const;

	void SetGlobalArea(ECameraPosition NewCameraPosition);
	void SetDoorsOpen(bool bOpen = true);

	ALevelSequenceActor* GetLevelSequenceActor(ELevelSequenceType SequenceType) const;

protected:

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY(ReplicatedUsing = OnRep_GlobalArea)
	ECameraPosition GlobalArea;

	UFUNCTION()
	void OnRep_GlobalArea();

	UPROPERTY(ReplicatedUsing = OnRep_DoorsOpen)
	bool bDoorsOpen;

	UFUNCTION()
	void OnRep_DoorsOpen();

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
