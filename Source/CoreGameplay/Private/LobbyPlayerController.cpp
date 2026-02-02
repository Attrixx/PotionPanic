// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "CustomGameViewportClient.h"

#include "Kismet/GameplayStatics.h"

#include "Net/UnrealNetwork.h"   
#include "LobbyPlayerPreview.h"
ALobbyPlayerController::ALobbyPlayerController()
{
}

void ALobbyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyPlayerController, MyPreviewActor);
}
void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<UCustomGameViewportClient> ViewportClient = Cast<UCustomGameViewportClient>(GetWorld()->GetGameViewport());
	if (ViewportClient)
	{
		ViewportClient->OnLocalPlayerJoinRequest.AddDynamic(this, &ALobbyPlayerController::HandleJoinRequest);
	}
}

void ALobbyPlayerController::SetupInputComponent()
{
}

void ALobbyPlayerController::HandleJoinRequest(int32 ControllerId)
{
	UE_LOG(LogTemp, Log, TEXT("Join request received for ControllerId: %d"), ControllerId);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Join request received for ControllerId: %d"), ControllerId));
	}

	// TODO: Handle two request at the same time
	PendingControllerId = ControllerId;

	ServerRequestNewLocalPlayer();
}

void ALobbyPlayerController::ServerRequestNewLocalPlayer_Implementation()
{
	TObjectPtr<ALobbyGameMode> LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode && LobbyGameMode->CanHandleNewPlayer())
	{
		ClientAuthorizeNewLocalPlayer();
	}
}

void ALobbyPlayerController::ClientAuthorizeNewLocalPlayer_Implementation()
{
	UGameplayStatics::CreatePlayer(this->GetWorld(), PendingControllerId, true);
}
