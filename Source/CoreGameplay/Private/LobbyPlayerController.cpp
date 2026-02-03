// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"
#include "LobbyPlayerState.h"
#include "CustomGameViewportClient.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"

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

void ALobbyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	// Get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// Add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("InputMappingContext added for ControllerId: %d"), GetLocalPlayer()->GetControllerId()));
		}
	}
}

void ALobbyPlayerController::OnNetCleanup(UNetConnection *Connection)
{
	if (GetLocalPlayer() && GetWorld())
	{
		UGameInstance *GI = GetWorld()->GetGameInstance();
		if (GI)
		{
			if (GetLocalPlayer()->GetControllerId() > 0)
			{
				GI->RemoveLocalPlayer(GetLocalPlayer());
			}
		}
	}

	Super::OnNetCleanup(Connection);
}

void ALobbyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(JoinAction, ETriggerEvent::Started, this, &ThisClass::Join);
		EnhancedInputComponent->BindAction(LeaveAction, ETriggerEvent::Started, this, &ThisClass::Leave);
		EnhancedInputComponent->BindAction(InviteAction, ETriggerEvent::Started, this, &ThisClass::Invite);
	}
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
	APlayerController* NewPC = UGameplayStatics::CreatePlayer(this->GetWorld(), PendingControllerId, true);
}

void ALobbyPlayerController::ServerLocalPlayerLeave_Implementation()
{
	this->Destroy();
}

void ALobbyPlayerController::SetLobbyPlayerColor(FColor NewColor)
{
	if (ALobbyPlayerState* LobbyPS = GetPlayerState<ALobbyPlayerState>())
	{
		LobbyPS->SetPlayerColor(NewColor);
	}
}

void ALobbyPlayerController::SetLobbyReady(bool bIsReady)
{
	if (ALobbyPlayerState* LobbyPS = GetPlayerState<ALobbyPlayerState>())
	{
		LobbyPS->SetIsReady(bIsReady);
	}

	if (HasAuthority())
	{
		if (ALobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALobbyGameMode>())
		{
			GM->CheckGameStart();
		}
	}
}

void ALobbyPlayerController::Join(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, TEXT("Join action triggered."));
}

void ALobbyPlayerController::Leave(const FInputActionValue& Value)
{
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (LocalPlayer->GetControllerId() == 0)
		{
			// Primary player leaves: Disconnect and return to main menu
			UGameplayStatics::OpenLevel(this, FName("TestMap"));
		}
		else
		{
			// Secondary player leaves: Destroy controller from server then remove from local players from OnNetCleanup
			ServerLocalPlayerLeave();
		}
	}
}

void ALobbyPlayerController::Invite(const FInputActionValue& Value)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface())
	{
		ExternalUI->ShowFriendsUI(0);
	}
}
