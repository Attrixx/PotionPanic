// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/PotionPanicPlayerController.h"
#include "Core/PotionPanicCharacter.h"
#include "Core/GameplayAbilitySystem/PotionPanicTags.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Kismet/KismetMathLibrary.h"
#include "ScoreSystem/ScoreHUDWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

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
			}
		}
	}
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
