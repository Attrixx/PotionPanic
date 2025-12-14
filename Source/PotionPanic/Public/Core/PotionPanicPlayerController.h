// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PotionPanicPlayerController.generated.h"

class APotionPanicCharacter;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
struct FGameplayTag;
enum class EOrderRoundResult : uint8;
struct FClientOrderEntry;
class AOrderClient;

class UScoreHUDWidget;
class UOrderHUDWidget;
class UMainMenuWidget;
class UEndMenuWidget;
class UUserWidget;
class UScoreWorldSubsystem;
class UCommandeManagerWorldSubsystem;

UCLASS()
class POTIONPANIC_API APotionPanicPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	APotionPanicPlayerController();

protected:

	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void SetupInputComponent() override;

private:

	float DashZForce = 50.f;

protected:

	APotionPanicCharacter* PotionPanicCharacter;

	/*
	*	INPUT MAPPING
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* PickUpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* QTEAction1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* QTEAction2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* QTEAction3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* QTEAction4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float RotationSpeedScale = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float DashCooldown = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float DashStrength = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UScoreHUDWidget> ScoreWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UOrderHUDWidget> OrderWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMainMenuWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEndMenuWidget> EndMenuWidgetClass;

	UPROPERTY()
	UScoreHUDWidget* ScoreWidgetInstance = nullptr;

	UPROPERTY()
	UOrderHUDWidget* OrderWidgetInstance = nullptr;

	UPROPERTY()
	UMainMenuWidget* MainMenuWidgetInstance = nullptr;

	UPROPERTY()
	UEndMenuWidget* EndMenuWidgetInstance = nullptr;

	UPROPERTY()
	UScoreWorldSubsystem* ScoreSubsystem = nullptr;

	UPROPERTY()
	UCommandeManagerWorldSubsystem* CommandeManagerSubsystem = nullptr;

	UPROPERTY()
	TWeakObjectPtr<AOrderClient> BoundOrderClient;

	bool bGameStarted = false;
protected:

	/*
	* Input related methods
	*/
	void Move(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void PickUp(const FInputActionValue& Value);
	void Dash(const FInputActionValue& Value);
	void QuickTimeEvent1(const FInputActionValue& Value);
	void QuickTimeEvent2(const FInputActionValue& Value);
	void QuickTimeEvent3(const FInputActionValue& Value);
	void QuickTimeEvent4(const FInputActionValue& Value);

	void OnQuickTimeEventInput(const FGameplayTag& KeyTag) const;

	bool ActivateAbility(const FGameplayTag& AbilityTag) const;

public:

	void ForceDropOnHit();

private:
	void ShowMainMenu();
	void HideMainMenu();
	void ShowEndMenu(bool bIsVictory, int32 Score);
	void HideEndMenu();

	UFUNCTION()
	void HandlePlayRequested();

	UFUNCTION()
	void HandleReplayRequested();

	UFUNCTION()
	void HandleReturnToMenuRequested();

	UFUNCTION()
	void HandleRoundEnded(AOrderClient* Client, EOrderRoundResult Result, int32 SuccessCount);

	void StartAllRounds();
	AOrderClient* FindOrCreateOrderClient() const;
	void ResetScore();
	int32 GetCurrentScore() const;

	void ApplyInputModeUI(UUserWidget* FocusWidget);
	void ApplyInputModeGame();

	void BindToOrderClient(AOrderClient* Client);
	void UnbindFromOrderClient();

	UFUNCTION()
	void HandleOrderStarted(const FClientOrderEntry& Order);

	UFUNCTION()
	void HandleOrderUpdated(const FClientOrderEntry& Order, float RemainingTime, bool bIsActive);

	UFUNCTION()
	void HandleOrderFinished(AOrderClient* Client, const FClientOrderEntry& Order, bool bSuccess);
	
};
