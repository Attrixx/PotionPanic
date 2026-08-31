// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerState.h"
#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "AlchemistBase.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, PlayerInfo);
	DOREPLIFETIME(ALobbyPlayerState, PreviewActor);
}

void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();
	OnPlayerInfoChanged.Broadcast();
	OnPawnSet.AddDynamic(this, &ALobbyPlayerState::HandlePawnSet);
}

void ALobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnPlayerInfoChanged.Broadcast();
}

void ALobbyPlayerState::Server_SetPlayerColor_Implementation(EAlchemistColor NewColor)
{
	PlayerInfo.PlayerColor = NewColor;
	OnRep_PlayerInfo();

	// Notify the MPC on the server so all materials using it update immediately.
	if (ALobbyGameState* LobbyGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGS->UpdatePlayerColorInMPC(PlayerInfo.PlayerIndex, NewColor);
	}
}

FColor ALobbyPlayerState::GetPlayerColor() const
{
	FColor PlayerColor = FColor::White;

	UWorld* World = GetWorld();
	if (!IsValid(World)) return PlayerColor;

	ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>();
	if (!IsValid(LobbyGS)) return PlayerColor;

	if (UAlchemistCustomizationAsset* CustomizationData = LobbyGS->GetCustomizationData())
	{
		PlayerColor = CustomizationData->GetColor(PlayerInfo.PlayerColor);
	}
	return PlayerColor;
}

void ALobbyPlayerState::ApplyColorToCharacter(AAlchemistBase* Character) const
{
	if (!IsValid(Character)) return;

	USkeletalMesh* MeshToUse = nullptr;
	FColor ColorToUse = FColor::White;

	if (ALobbyGameState* LobbyGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		if (UAlchemistCustomizationAsset* CustomizationData = LobbyGS->GetCustomizationData())
		{
			MeshToUse = CustomizationData->GetMesh(PlayerInfo.PlayerColor);
			ColorToUse = CustomizationData->GetColor(PlayerInfo.PlayerColor);
		}
	}

	Character->ApplyCustomization(MeshToUse, ColorToUse);
}

void ALobbyPlayerState::SetPlayerIndex(int32 Index)
{
	PlayerInfo.PlayerIndex = Index;
	OnRep_PlayerInfo();
}

void ALobbyPlayerState::SetIsHost(bool bHost)
{
	PlayerInfo.bIsHost = bHost;
	OnRep_PlayerInfo();
}

void ALobbyPlayerState::SetPreviewActor(AAlchemistBase* InPreviewActor)
{
	if (HasAuthority())
	{
		PreviewActor = InPreviewActor;
		OnRep_PreviewActor();
	}
}

void ALobbyPlayerState::OnRep_PreviewActor()
{
	if (IsValid(PreviewActor))
	{
		ApplyColorToCharacter(PreviewActor);
		PreviewActor->SetPlayerStencilIndex(PlayerInfo.PlayerIndex + 1 + 4); // Stencil from 5 to 8 are player colors, but without "x-ray"
	}
}

void ALobbyPlayerState::Server_SetIsReady_Implementation(bool bNewReady)
{
	PlayerInfo.bIsReady = bNewReady;
	OnRep_PlayerInfo();
}

void ALobbyPlayerState::Server_OnStartupSequenceFinished_Implementation()
{
	if (ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
	{
		LobbyGameMode->OnPlayerEndedStartupSequence(Cast<ACharacter>(GetPawn()));
	}
}

void ALobbyPlayerState::OnRep_PlayerInfo()
{
	OnPlayerInfoChanged.Broadcast();
	OnPlayerColorChanged.Broadcast(PlayerInfo.PlayerColor);

	// Stencil value: PlayerIndex + 1 (stencil 0 means "no custom depth", 1-4 identify players)
	const int32 StencilValue = PlayerInfo.PlayerIndex + 1;

	if (AAlchemistBase* Character = Cast<AAlchemistBase>(GetPawn()))
	{
		ApplyColorToCharacter(Character);
		Character->SetPlayerStencilIndex(StencilValue);
	}

	if (IsValid(PreviewActor))
	{
		ApplyColorToCharacter(PreviewActor);
		PreviewActor->SetPlayerStencilIndex(StencilValue);
	}

	// Update the MPC on clients as well (OnRep is called on clients; UpdatePlayerColorInMPC works with local MPCI)
	if (ALobbyGameState* LobbyGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LobbyGS->UpdatePlayerColorInMPC(PlayerInfo.PlayerIndex, PlayerInfo.PlayerColor);
	}
}

void ALobbyPlayerState::HandlePawnSet(APlayerState* Player, APawn* NewPawn, APawn* OldPawn)
{
	if (AAlchemistBase* Character = Cast<AAlchemistBase>(NewPawn))
	{
		ApplyColorToCharacter(Character);
		Character->SetPlayerStencilIndex(PlayerInfo.PlayerIndex + 1);
	}
}
