// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PotionPanicPlayerController.h"
#include "Core/PotionPanicCharacter.h"
#include "Core/GameplayAbilitySystem/PotionPanicTags.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "ScoreSystem/ScoreHUDWidget.h"
#include "ScoreSystem/ScoreWorldSubsystem.h"
#include "OrderSystem/CommandeManagerWorldSubsystem.h"
#include "OrderSystem/OrderClient.h"
#include "UserInterface/OrderHUDWidget.h"
#include "UserInterface/MainMenuWidget.h"
#include "UserInterface/EndMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

APotionPanicPlayerController::APotionPanicPlayerController()
{
	OrderWidgetClass = UOrderHUDWidget::StaticClass();
}

void APotionPanicPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APotionPanicPlayerController, ReplicatedOrderClient);
}

void APotionPanicPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PotionPanicCharacter = Cast<APotionPanicCharacter>(GetCharacter());

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(InputMappingContext, 0);

		if (UEnhancedInputUserSettings* UserSettings = Subsystem->GetUserSettings())
		{
			if (!UserSettings->IsMappingContextRegistered(InputMappingContext))
			{
				UserSettings->RegisterInputMappingContext(InputMappingContext);
			}
		}
	}

	if (IsLocalController())
	{
		if (ScoreWidgetClass)
		{
			ScoreWidgetInstance = CreateWidget<UScoreHUDWidget>(this, ScoreWidgetClass);
			if (ScoreWidgetInstance)
			{
				ScoreWidgetInstance->AddToViewport();
				ScoreWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
			}
		}

		if (OrderWidgetClass)
		{
			OrderWidgetInstance = CreateWidget<UOrderHUDWidget>(this, OrderWidgetClass);
			if (OrderWidgetInstance)
			{
				OrderWidgetInstance->AddToViewport();
				OrderWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		ScoreSubsystem = World->GetSubsystem<UScoreWorldSubsystem>();
		CommandeManagerSubsystem = World->GetSubsystem<UCommandeManagerWorldSubsystem>();
	}

	if (HasAuthority())
	{
		if (CommandeManagerSubsystem)
		{
			CommandeManagerSubsystem->OnRoundEnded.AddDynamic(this, &ThisClass::HandleRoundEnded);
		}

		if (ScoreSubsystem)
		{
			ScoreSubsystem->OnScoreChanged.AddDynamic(this, &ThisClass::HandleServerScoreChanged);

			// Sync initial score to the owning client (for late joiners).
			ClientScoreUpdated(ScoreSubsystem->GetScore());
		}

		// Late join support: if an order client already exists, replicate the reference immediately.
		if (!ReplicatedOrderClient && GetWorld())
		{
			TArray<AActor*> Clients;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOrderClient::StaticClass(), Clients);
			if (Clients.Num() > 0)
			{
				ReplicatedOrderClient = Cast<AOrderClient>(Clients[0]);
				BindToOrderClient(ReplicatedOrderClient);
			}
		}
	}

	ShowMainMenu();
}

void APotionPanicPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction,		ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(InteractAction,	ETriggerEvent::Triggered, this, &ThisClass::Interact);
		EnhancedInputComponent->BindAction(PickUpAction,	ETriggerEvent::Triggered, this, &ThisClass::PickUp);
		EnhancedInputComponent->BindAction(DashAction,		ETriggerEvent::Triggered, this, &ThisClass::Dash);
		EnhancedInputComponent->BindAction(QTEAction1,		ETriggerEvent::Triggered, this, &ThisClass::QuickTimeEvent1);
		EnhancedInputComponent->BindAction(QTEAction2,		ETriggerEvent::Triggered, this, &ThisClass::QuickTimeEvent2);
		EnhancedInputComponent->BindAction(QTEAction3,		ETriggerEvent::Triggered, this, &ThisClass::QuickTimeEvent3);
		EnhancedInputComponent->BindAction(QTEAction4,		ETriggerEvent::Triggered, this, &ThisClass::QuickTimeEvent4);
	}
}

void APotionPanicPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CommandeManagerSubsystem && HasAuthority())
	{
		CommandeManagerSubsystem->OnRoundEnded.RemoveDynamic(this, &ThisClass::HandleRoundEnded);
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->OnPlayRequested.RemoveAll(this);
	}

	if (ScoreSubsystem && HasAuthority())
	{
		ScoreSubsystem->OnScoreChanged.RemoveDynamic(this, &ThisClass::HandleServerScoreChanged);
	}

	if (EndMenuWidgetInstance)
	{
		EndMenuWidgetInstance->OnReplayRequested.RemoveAll(this);
		EndMenuWidgetInstance->OnReturnToMenuRequested.RemoveAll(this);
	}

	UnbindFromOrderClient();

	Super::EndPlay(EndPlayReason);
}

void APotionPanicPlayerController::Move(const FInputActionValue& Value)
{
	ACharacter* CurrentCharacter = GetCharacter();
	if (CurrentCharacter == nullptr) return;
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsValid(PotionPanicCharacter))
	{
		UAbilitySystemComponent* AbilitySystemComponent = PotionPanicCharacter->GetAbilitySystemComponent();
		if (IsValid(AbilitySystemComponent))
		{
			if (AbilitySystemComponent->HasMatchingGameplayTag(PotionPanicTags::Character::State::UsingStation))
			{
				return;
			}
		}
	}

	CurrentCharacter->AddMovementInput(FVector::ForwardVector, MovementVector.Y);
	CurrentCharacter->AddMovementInput(FVector::RightVector, MovementVector.X);

	const float RotationAngle = FMath::Atan2(-MovementVector.Y, MovementVector.X);
	const float TargetYaw = FMath::RadiansToDegrees(RotationAngle) + 90.f;
	const float CurrentYaw = CurrentCharacter->GetActorRotation().Yaw;
	const float DeltaYaw = UKismetMathLibrary::NormalizeAxis(TargetYaw - CurrentYaw);
	float NormalizedYawInput = FMath::Clamp(DeltaYaw, -1.f * RotationSpeedScale, 1.f * RotationSpeedScale);
	CurrentCharacter->AddControllerYawInput(NormalizedYawInput);
}

void APotionPanicPlayerController::Interact(const FInputActionValue& Value)
{
	if (!IsValid(PotionPanicCharacter)) return;
	if (ActivateAbility(PotionPanicTags::Abilities::Throw)) return;
	ActivateAbility(PotionPanicTags::Abilities::Interact);
}

void APotionPanicPlayerController::PickUp(const FInputActionValue& Value)
{
	if (!IsValid(PotionPanicCharacter)) return;

	if (ActivateAbility(PotionPanicTags::Abilities::PickUp)) return;
	ActivateAbility(PotionPanicTags::Abilities::Drop);
}

void APotionPanicPlayerController::Dash(const FInputActionValue& Value)
{
	if (!IsValid(PotionPanicCharacter)) return;

	ActivateAbility(PotionPanicTags::Abilities::Dash);

	/*bCanDash = false;
	PotionPanicCharacter->OnDashStart();

	PotionPanicCharacter->LaunchCharacter(PotionPanicCharacter->GetActorForwardVector() * DashStrength + FVector(0.f, 0.f, DashZForce), false, false);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]() { bCanDash = true; PotionPanicCharacter->OnDashEnd(); }, DashCooldown, false);*/
}

void APotionPanicPlayerController::QuickTimeEvent1(const FInputActionValue& Value)
{
	OnQuickTimeEventInput(FGameplayTag::RequestGameplayTag(FName("Keys.Triangle")));
}

void APotionPanicPlayerController::QuickTimeEvent2(const FInputActionValue& Value)
{
	OnQuickTimeEventInput(FGameplayTag::RequestGameplayTag(FName("Keys.Square")));
}

void APotionPanicPlayerController::QuickTimeEvent3(const FInputActionValue& Value)
{
	OnQuickTimeEventInput(FGameplayTag::RequestGameplayTag(FName("Keys.Circle")));
}

void APotionPanicPlayerController::QuickTimeEvent4(const FInputActionValue& Value)
{
	OnQuickTimeEventInput(FGameplayTag::RequestGameplayTag(FName("Keys.Cross")));
}

void APotionPanicPlayerController::OnQuickTimeEventInput(const FGameplayTag& KeyTag) const
{
	if (!IsValid(PotionPanicCharacter)) return;
	UAbilitySystemComponent* AbilitySystemComponent = PotionPanicCharacter->GetAbilitySystemComponent();
	if (!IsValid(AbilitySystemComponent)) return;
	if (!AbilitySystemComponent->HasMatchingGameplayTag(PotionPanicTags::Character::State::UsingStation)) return;
	
	//FGameplayEventData EventData;
	//EventData.EventTag = KeyTag;
	//EventData.Instigator = PotionPanicCharacter;
	//UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PotionPanicCharacter->GetBestInteractableActor(), EventData.EventTag, EventData);
	
	PotionPanicCharacter->Server_SubmitQTEInput(KeyTag, PotionPanicCharacter->GetBestInteractableActor());
}

bool APotionPanicPlayerController::ActivateAbility(const FGameplayTag& AbilityTag) const
{
	if (!IsValid(PotionPanicCharacter)) return false;

	UAbilitySystemComponent* AbilitySystemComponent = PotionPanicCharacter->GetAbilitySystemComponent();
	if (!IsValid(AbilitySystemComponent)) return false;

	return AbilitySystemComponent->TryActivateAbilitiesByTag(AbilityTag.GetSingleTagContainer());
}

void APotionPanicPlayerController::ForceDropOnHit()
{
	ActivateAbility(PotionPanicTags::Abilities::Drop);
}

void APotionPanicPlayerController::ShowMainMenu()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!MainMenuWidgetInstance && MainMenuWidgetClass)
	{
		MainMenuWidgetInstance = CreateWidget<UMainMenuWidget>(this, MainMenuWidgetClass);
		if (MainMenuWidgetInstance)
		{
			MainMenuWidgetInstance->OnPlayRequested.AddDynamic(this, &ThisClass::HandlePlayRequested);
		}
	}

	if (MainMenuWidgetInstance && !MainMenuWidgetInstance->IsInViewport())
	{
		MainMenuWidgetInstance->AddToViewport();
	}

	if (ScoreWidgetInstance)
	{
		ScoreWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	if (OrderWidgetInstance)
	{
		OrderWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	ApplyInputModeUI(MainMenuWidgetInstance);
}

void APotionPanicPlayerController::HideMainMenu()
{
	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->RemoveFromParent();
	}
}

void APotionPanicPlayerController::ShowEndMenu(bool bIsVictory, int32 Score)
{
	if (!IsLocalController())
	{
		return;
	}

	if (!EndMenuWidgetInstance && EndMenuWidgetClass)
	{
		EndMenuWidgetInstance = CreateWidget<UEndMenuWidget>(this, EndMenuWidgetClass);
		if (EndMenuWidgetInstance)
		{
			EndMenuWidgetInstance->OnReplayRequested.AddDynamic(this, &ThisClass::HandleReplayRequested);
			EndMenuWidgetInstance->OnReturnToMenuRequested.AddDynamic(this, &ThisClass::HandleReturnToMenuRequested);
		}
	}

	if (EndMenuWidgetInstance)
	{
		EndMenuWidgetInstance->SetEndState(bIsVictory, Score);

		if (!EndMenuWidgetInstance->IsInViewport())
		{
			EndMenuWidgetInstance->AddToViewport();
		}
	}

	if (ScoreWidgetInstance)
	{
		ScoreWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	if (OrderWidgetInstance)
	{
		OrderWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}

	ApplyInputModeUI(EndMenuWidgetInstance);
}

void APotionPanicPlayerController::HideEndMenu()
{
	if (EndMenuWidgetInstance)
	{
		EndMenuWidgetInstance->RemoveFromParent();
	}
}

void APotionPanicPlayerController::HandlePlayRequested()
{
	HideMainMenu();
	HideEndMenu();
	ApplyInputModeGame();
	bGameStarted = true;

	if (HasAuthority())
	{
		ResetScore();
		StartAllRounds();
	}
	else
	{
		ServerHandlePlayRequested();
	}
}

void APotionPanicPlayerController::HandleReplayRequested()
{
	HideEndMenu();
	ApplyInputModeGame();
	bGameStarted = true;

	if (HasAuthority())
	{
		ResetScore();
		StartAllRounds();
	}
	else
	{
		ServerHandleReplayRequested();
	}
}

void APotionPanicPlayerController::HandleReturnToMenuRequested()
{
	HideEndMenu();
	ShowMainMenu();
	bGameStarted = false;

	if (HasAuthority())
	{
		ResetScore();
	}
	else
	{
		ServerHandleReturnToMenuRequested();
	}
}

void APotionPanicPlayerController::ServerHandlePlayRequested_Implementation()
{
	ResetScore();
	StartAllRounds();
	bGameStarted = true;
}

void APotionPanicPlayerController::ServerHandleReplayRequested_Implementation()
{
	ResetScore();
	StartAllRounds();
	bGameStarted = true;
}

void APotionPanicPlayerController::ServerHandleReturnToMenuRequested_Implementation()
{
	ResetScore();
	bGameStarted = false;
}

void APotionPanicPlayerController::HandleRoundEnded(AOrderClient* Client, EOrderRoundResult Result, int32 SuccessCount)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!bGameStarted)
	{
		return;
	}

	const bool bIsVictory = (Result != EOrderRoundResult::Lose);
	const int32 CurrentScore = GetCurrentScore();
	if (IsLocalController())
	{
		ShowEndMenu(bIsVictory, CurrentScore);
	}
	else
	{
		ClientHandleRoundEnded(bIsVictory, CurrentScore);
	}
	bGameStarted = false;
}

void APotionPanicPlayerController::StartAllRounds()
{
	if (!HasAuthority() || !CommandeManagerSubsystem || !GetWorld())
	{
		return;
	}

	TArray<AActor*> Clients;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOrderClient::StaticClass(), Clients);

	if (Clients.Num() == 0)
	{
		if (AOrderClient* NewClient = FindOrCreateOrderClient())
		{
			Clients.Add(NewClient);
		}
	}

	AOrderClient* FirstClient = Clients.Num() > 0 ? Cast<AOrderClient>(Clients[0]) : nullptr;
	if (FirstClient)
	{
		BindToOrderClient(FirstClient);
	}

	for (AActor* Actor : Clients)
	{
		if (AOrderClient* Client = Cast<AOrderClient>(Actor))
		{
			CommandeManagerSubsystem->StartRound(Client);
		}
	}
}

AOrderClient* APotionPanicPlayerController::FindOrCreateOrderClient() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AOrderClient* NewClient = GetWorld()->SpawnActor<AOrderClient>(AOrderClient::StaticClass(), FTransform::Identity, Params);
	if (NewClient)
	{
		NewClient->SetActorHiddenInGame(true);
		NewClient->SetActorEnableCollision(false);
		NewClient->SetActorTickEnabled(false);
	}

	return NewClient;
}

void APotionPanicPlayerController::OnRep_OrderClient()
{
	BindToOrderClient(ReplicatedOrderClient);
}

void APotionPanicPlayerController::BindToOrderClient(AOrderClient* Client)
{
	if (BoundOrderClient.Get() == Client)
	{
		return;
	}

	UnbindFromOrderClient();

	if (!Client)
	{
		if (OrderWidgetInstance)
		{
			OrderWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	if (HasAuthority())
	{
		ReplicatedOrderClient = Client;
	}

	BoundOrderClient = Client;
	Client->OnOrderStarted.AddDynamic(this, &ThisClass::HandleOrderStarted);
	Client->OnOrderUpdated.AddDynamic(this, &ThisClass::HandleOrderUpdated);
	Client->OnOrderFinished.AddDynamic(this, &ThisClass::HandleOrderFinished);

	if (OrderWidgetInstance && Client->HasActiveOrder())
	{
		OrderWidgetInstance->UpdateOrder(Client->GetCurrentOrder(), Client->GetRemainingTime(), true);
	}
}

void APotionPanicPlayerController::UnbindFromOrderClient()
{
	if (AOrderClient* Client = BoundOrderClient.Get())
	{
		Client->OnOrderStarted.RemoveDynamic(this, &ThisClass::HandleOrderStarted);
		Client->OnOrderUpdated.RemoveDynamic(this, &ThisClass::HandleOrderUpdated);
		Client->OnOrderFinished.RemoveDynamic(this, &ThisClass::HandleOrderFinished);
	}

	BoundOrderClient.Reset();
	if (HasAuthority())
	{
		ReplicatedOrderClient = nullptr;
	}

	if (OrderWidgetInstance)
	{
		OrderWidgetInstance->SetVisibility(ESlateVisibility::Hidden);
	}
}

void APotionPanicPlayerController::HandleOrderStarted(const FClientOrderEntry& Order)
{
	const float Remaining = BoundOrderClient.IsValid()
		? BoundOrderClient->GetRemainingTime()
		: Order.Duration;

	HandleOrderUpdated(Order, Remaining, true);
}

void APotionPanicPlayerController::HandleOrderUpdated(const FClientOrderEntry& Order, float RemainingTime, bool bIsActive)
{
	if (OrderWidgetInstance)
	{
		OrderWidgetInstance->UpdateOrder(Order, RemainingTime, bIsActive);
	}
}

void APotionPanicPlayerController::HandleOrderFinished(AOrderClient* Client, const FClientOrderEntry& Order, bool bSuccess)
{
	if (OrderWidgetInstance)
	{
		OrderWidgetInstance->ShowResult(bSuccess);
	}
}

void APotionPanicPlayerController::ResetScore()
{
	if (ScoreSubsystem)
	{
		ScoreSubsystem->ResetScore();
	}
}

int32 APotionPanicPlayerController::GetCurrentScore() const
{
	return ScoreSubsystem ? ScoreSubsystem->GetScore() : 0;
}

void APotionPanicPlayerController::HandleServerScoreChanged(int32 NewScore)
{
	// Propagate the new score to the owning client so its local UI can update.
	ClientScoreUpdated(NewScore);
}

void APotionPanicPlayerController::ClientScoreUpdated_Implementation(int32 NewScore)
{
	if (ScoreSubsystem)
	{
		ScoreSubsystem->SetScore(NewScore);
	}
}

void APotionPanicPlayerController::ClientHandleRoundEnded_Implementation(bool bIsVictory, int32 Score)
{
	ShowEndMenu(bIsVictory, Score);
	bGameStarted = false;
}

void APotionPanicPlayerController::ApplyInputModeUI(UUserWidget* FocusWidget)
{
	UGameplayStatics::SetGamePaused(this, true);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(FocusWidget ? FocusWidget->TakeWidget() : TSharedPtr<SWidget>());
	SetInputMode(InputMode);

	bShowMouseCursor = true;
}

void APotionPanicPlayerController::ApplyInputModeGame()
{
	HideMainMenu();
	HideEndMenu();

	UGameplayStatics::SetGamePaused(this, false);

	if (ScoreWidgetInstance)
	{
		ScoreWidgetInstance->SetVisibility(ESlateVisibility::Visible);
	}

	if (OrderWidgetInstance)
	{
		const bool bShouldShowOrder = BoundOrderClient.IsValid() && BoundOrderClient->HasActiveOrder();
		OrderWidgetInstance->SetVisibility(bShouldShowOrder ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
}
