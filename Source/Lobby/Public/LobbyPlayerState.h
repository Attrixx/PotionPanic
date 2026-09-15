// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PotionPanicPlayerState.h"
#include "LobbyGameState.h"
#include "LobbyPlayerState.generated.h"

class AAlchemistBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLobbyPlayerStateUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyPlayerColorChanged, EAlchemistColor, NewColor);


USTRUCT(BlueprintType)
struct FLobbyPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAlchemistColor PlayerColor = EAlchemistColor::Blue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCouchCoopPlayer = false;
};


/**
 * 
 */
UCLASS()
class LOBBY_API ALobbyPlayerState : public APotionPanicPlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* NewPlayerState) override;

protected:

	void BeginPlay() override;
	void OnRep_PlayerName() override;

	// The Lobby drives its preview / pawn visuals from FLobbyPlayerInfo below, so opt out of the
	// base auto-apply to avoid doing the work twice.
	virtual bool ShouldAutoApplyCustomization() const override { return false; }

public:

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerColor(EAlchemistColor NewColor);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	FColor GetPlayerColor() const;

	// Not an RPC: only the server should call this directly.
	void SetPlayerIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	int32 GetPlayerIndex() const { return PlayerInfo.PlayerIndex; }

	// Not an RPC, only server should be allowed to set this
	void SetIsHost(bool bHost);

	UFUNCTION(Server, Reliable)
	void Server_SetIsReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsReady() const { return PlayerInfo.bIsReady; }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsHost() const { return PlayerInfo.bIsHost; }

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool IsCouchCoopPlayer() const { return PlayerInfo.bIsCouchCoopPlayer; }

	UFUNCTION(Server, Reliable)
	void Server_OnStartupSequenceFinished();

protected:

	UPROPERTY(ReplicatedUsing=OnRep_PlayerInfo)
	FLobbyPlayerInfo PlayerInfo;

	UFUNCTION()
	void OnRep_PlayerInfo();

	UPROPERTY(ReplicatedUsing=OnRep_PreviewActor)
	TObjectPtr<AAlchemistBase> PreviewActor;

	UFUNCTION()
	void OnRep_PreviewActor();

public:
	void SetPreviewActor(AAlchemistBase* InPreviewActor);
	AAlchemistBase* GetPreviewActor() const { return PreviewActor; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerStateUpdated OnPlayerInfoChanged;
	UPROPERTY(BlueprintAssignable, Category = "Lobby")
	FOnLobbyPlayerColorChanged OnPlayerColorChanged;

protected:
	UFUNCTION()
	void HandlePawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn);
	
	void ApplyColorToCharacter(AAlchemistBase* Character) const;
};
