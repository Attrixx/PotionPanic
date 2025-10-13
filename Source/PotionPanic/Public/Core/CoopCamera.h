#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "CoopCamera.generated.h"

class USpringArmComponent;

UCLASS()
class ACoopCamera : public ACameraActor
{
    GENERATED_BODY()

public:
    ACoopCamera();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "CoopCamera")
    USpringArmComponent* SpringArm = nullptr;

    UPROPERTY(EditAnywhere, Category = "CoopCamera")
    float Padding = 300.f;

    UPROPERTY(EditAnywhere, Category = "CoopCamera")
    float LocationLerpSpeed = 6.f;

    UPROPERTY(EditAnywhere, Category = "CoopCamera")
    float ZoomLerpSpeed = 4.f;

    UPROPERTY(EditAnywhere, Category = "CoopCamera")
    bool bDebugDraw = false;

    bool GatherPlayerPositions(TArray<FVector2D>& OutXY, FVector& OutAvg3D) const;
    float ComputeMaxPairDistance(const TArray<FVector2D>& XY, FVector2D* OutA = nullptr, FVector2D* OutB = nullptr) const;
};
