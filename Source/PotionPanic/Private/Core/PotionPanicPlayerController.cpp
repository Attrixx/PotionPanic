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
#include "UserInterface/MainMenuWidget.h"
#include "UserInterface/EndMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

APotionPanicPlayerController::APotionPanicPlayerController()
{
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
	}

	if (UWorld* World = GetWorld())
	{
		ScoreSubsystem = World->GetSubsystem<UScoreWorldSubsystem>();
		CommandeManagerSubsystem = World->GetSubsystem<UCommandeManagerWorldSubsystem>();
	}

	if (CommandeManagerSubsystem)
	{
		CommandeManagerSubsystem->OnRoundEnded.AddDynamic(this, &ThisClass::HandleRoundEnded);
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
	}
}

void APotionPanicPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CommandeManagerSubsystem)
	{
		CommandeManagerSubsystem->OnRoundEnded.RemoveDynamic(this, &ThisClass::HandleRoundEnded);
	}

	if (MainMenuWidgetInstance)
	{
		MainMenuWidgetInstance->OnPlayRequested.RemoveAll(this);
	}

	if (EndMenuWidgetInstance)
	{
		EndMenuWidgetInstance->OnReplayRequested.RemoveAll(this);
		EndMenuWidgetInstance->OnReturnToMenuRequested.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void APotionPanicPlayerController::Move(const FInputActionValue& Value)
{
	ACharacter* CurrentCharacter = GetCharacter();
	if (CurrentCharacter == nullptr) return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	CurrentCharacter->AddMovementInput(FVector::ForwardVector, MovementVector.Y);
	CurrentCharacter->AddMovementInput(FVector::RightVector, MovementVector.X);
	
	const float RotationAngle = FMath::Atan2(-MovementVector.Y, MovementVector.X);
	const float TargetYaw = FMath::RadiansToDegrees(RotationAngle) + 90.f;
	const float CurrentYaw = CurrentCharacter->GetActorRotation().Yaw;
	const float DeltaYaw = UKismetMathLibrary::NormalizeAxis(TargetYaw - CurrentYaw);
	float NormalizedYawInput = FMath::Clamp(DeltaYaw, -1.f * RotationSpeedScale, 1.f * RotationSpeedScale);
	CurrentCharacter->AddControllerYawInput(NormalizedYawInput);

	if (!IsValid(PotionPanicCharacter)) return;

	UAbilitySystemComponent* AbilitySystemComponent = PotionPanicCharacter->GetAbilitySystemComponent();
	if (!IsValid(AbilitySystemComponent)) return;

	if (!AbilitySystemComponent->HasMatchingGameplayTag(PotionPanicTags::Character::State::UsingStation)) return;

	FGameplayTag KeyTag;
	if (MovementVector.Y > 0.f) KeyTag = FGameplayTag::RequestGameplayTag(FName("Keys.Triangle"));
	else if (MovementVector.Y < 0.f) KeyTag = FGameplayTag::RequestGameplayTag(FName("Keys.Cross"));
	else if (MovementVector.X > 0.f) KeyTag = FGameplayTag::RequestGameplayTag(FName("Keys.Circle"));
	else if (MovementVector.X < 0.f) KeyTag = FGameplayTag::RequestGameplayTag(FName("Keys.Square"));
	else return;

	FGameplayEventData EventData;
	EventData.EventTag = KeyTag;
	EventData.Instigator = PotionPanicCharacter;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PotionPanicCharacter->GetBestInteractableActor(), EventData.EventTag, EventData);
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

	ResetScore();
	StartAllRounds();
	ApplyInputModeGame();
	bGameStarted = true;
}

void APotionPanicPlayerController::HandleReplayRequested()
{
	HideEndMenu();

	ResetScore();
	StartAllRounds();
	ApplyInputModeGame();
	bGameStarted = true;
}

void APotionPanicPlayerController::HandleReturnToMenuRequested()
{
	HideEndMenu();

	ResetScore();
	bGameStarted = false;
	ShowMainMenu();
}

void APotionPanicPlayerController::HandleRoundEnded(AOrderClient* Client, EOrderRoundResult Result, int32 SuccessCount)
{
	if (!bGameStarted)
	{
		return;
	}

	const bool bIsVictory = (Result != EOrderRoundResult::Lose);
	const int32 CurrentScore = GetCurrentScore();
	ShowEndMenu(bIsVictory, CurrentScore);
	bGameStarted = false;
}

void APotionPanicPlayerController::StartAllRounds()
{
	if (!CommandeManagerSubsystem || !GetWorld())
	{
		return;
	}

	TArray<AActor*> Clients;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AOrderClient::StaticClass(), Clients);
	for (AActor* Actor : Clients)
	{
		if (AOrderClient* Client = Cast<AOrderClient>(Actor))
		{
			CommandeManagerSubsystem->StartRound(Client);
		}
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

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
}
