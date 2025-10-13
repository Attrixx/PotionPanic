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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector Offset = FVector::ZeroVector;

	static void GetAllTargets(const UObject* WorldContext, TArray<FVector>& OutLocations, bool bCleanup = true);
	static int32 NumTargets(const UObject* WorldContext);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	static TArray<TWeakObjectPtr<UCamTargetComponent>> Registry;

	FVector ComputeLocation() const;
};
