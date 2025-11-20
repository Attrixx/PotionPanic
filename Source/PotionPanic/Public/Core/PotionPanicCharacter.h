#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "PotionPanicCharacter.generated.h"

class USphereComponent;
class AFlyingSocket;
class UCamTargetComponent;
class USocketComponent;
class USocketableComponent;
class UGameplayAbility;
class UGameplayEffect;
struct FGameplayTag;

UCLASS(Abstract)
class POTIONPANIC_API APotionPanicCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	APotionPanicCharacter();

protected:

	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	void EndPlay(EEndPlayReason::Type) override;

	UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	void PossessedBy(AController* NewController) override;
	void OnRep_PlayerState() override;

	void GiveStartupAbilities();
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass);
	void RemoveEffectByGrantedTag(const FGameplayTag& TagToRemove);
	void OnHeldChanged(USocketableComponent* OldHeld, USocketableComponent* NewHeld);

private:

	UFUNCTION() void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION() void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION() void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void SortInteractablesInRange();
	void SortSocketablesInRange();
	void SortSocketsInRange();

	float ComputeLocationScore(FVector Location);

	friend class APotionPanicPlayerController;


	void SetBestSocketable(USocketableComponent* NewBestSocketable);

public:

	UFUNCTION(BlueprintCallable)
	bool IsHolding() const;

	void PickupObject(USocketableComponent* ForceSocketable = nullptr);
	void DropObject();
	void ThrowHeldObject();
	void Interact();

	void OnDash();

protected:

	UPROPERTY(VisibleAnywhere, Category = "Potion Panic Character")
	TObjectPtr<USphereComponent> PickupRange;

	UPROPERTY(EditAnywhere, Category = "Potion Panic Character")
	TSubclassOf<AFlyingSocket> FlyingSocketClass;

	UPROPERTY(EditAnywhere, Category = "Potion Panic Character")
	float ObjectThrowSpeed;

	UPROPERTY(VisibleAnywhere, Category = "Potion Panic Character")
	TObjectPtr<USocketComponent> Socket;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> CanPickUpItemEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> CanInteractEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> CarryingEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> PickUpEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> ThrowEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> DropEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> DashEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Potion Panic|Effects")
	TSubclassOf<UGameplayEffect> DashingEffect;

private:

	TObjectPtr<UCamTargetComponent> CamTargetComponent;

	TMap<UActorComponent*, int32> InteractableActorsInRange;
	TMap<USocketableComponent*, int32> SocketableComponentsInRange;
	TMap<USocketComponent*, int32> SocketComponentsInRange;

	USocketComponent* BestSocket;
	USocketableComponent* BestSocketable;
	UActorComponent* BestInteractableComponent;

	bool bCanHitDash = true;

};
