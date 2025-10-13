#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CamTargetComponent.generated.h"

UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class UCamTargetComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCamTargetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bAutoRegister = true;
	void Register();
	void Unregister();

protected:
	void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	friend class ACoopCamera;
	static inline TArray<UCamTargetComponent*> Registry;
};
