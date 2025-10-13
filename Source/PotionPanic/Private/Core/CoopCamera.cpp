#include "Core/CoopCamera.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"

ACoopCamera::ACoopCamera()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(GetRootComponent());
    SpringArm->bDoCollisionTest = false;
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->SetUsingAbsoluteRotation(true);
    SpringArm->TargetArmLength = 1200.f;

    if (UCameraComponent* Cam = GetCameraComponent())
    {
        Cam->bUsePawnControlRotation = false;
        Cam->SetupAttachment(SpringArm);
    }
}

void ACoopCamera::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    SpringArm->SetWorldRotation(GetActorRotation());
}

void ACoopCamera::BeginPlay()
{
    Super::BeginPlay();
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetViewTarget(this);
    }
}

bool ACoopCamera::GatherPlayerPositions(TArray<FVector2D>& OutXY, FVector& OutAvg3D) const
{
    OutXY.Reset();
    FVector Sum = FVector::ZeroVector;
    int32 Count = 0;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        const APlayerController* PC = It->Get();
        if (!PC) continue;
        const APawn* P = PC->GetPawn();
        if (!P) continue;

        const FVector L = P->GetActorLocation();
        OutXY.Add(FVector2D(L.X, L.Y));
        Sum += L;
        ++Count;
    }
    if (Count == 0) return false;
    OutAvg3D = Sum / float(Count);
    return true;
}

float ACoopCamera::ComputeMaxPairDistance(const TArray<FVector2D>& XY, FVector2D* OutA, FVector2D* OutB) const
{
    if (XY.Num() < 2) return 0.f;
    float MaxD2 = 0.f; int32 ai = 0, bi = 1;
    for (int32 i = 0; i < XY.Num() - 1; ++i)
    {
        for (int32 j = i + 1; j < XY.Num(); ++j)
        {
            const float d2 = FVector2D::DistSquared(XY[i], XY[j]);
            if (d2 > MaxD2) { MaxD2 = d2; ai = i; bi = j; }
        }
    }
    if (OutA) *OutA = XY[ai];
    if (OutB) *OutB = XY[bi];
    return FMath::Sqrt(MaxD2);
}

void ACoopCamera::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UCameraComponent* Cam = GetCameraComponent();
    if (!Cam || !SpringArm) return;

    TArray<FVector2D> XY;
    FVector Avg3D;
    if (!GatherPlayerPositions(XY, Avg3D)) return;

    const FVector TargetPivot(Avg3D.X, Avg3D.Y, Avg3D.Z);
    const FVector NewPivot = FMath::VInterpTo(GetActorLocation(), TargetPivot, DeltaSeconds, LocationLerpSpeed);
    SetActorLocation(NewPivot);

    SpringArm->SetWorldRotation(GetActorRotation());

    FVector2D A, B;
    const float MaxPair = ComputeMaxPairDistance(XY, &A, &B);
    const float Spread = MaxPair + 2.f * Padding;

    if (Cam->ProjectionMode == ECameraProjectionMode::Orthographic)
    {
        const float TargetWidth = Spread;
        Cam->OrthoWidth = FMath::FInterpTo(Cam->OrthoWidth, TargetWidth, DeltaSeconds, ZoomLerpSpeed);
    }
    else
    {
        const float Aspect = Cam->AspectRatio;
        const float HFovRad = FMath::DegreesToRadians(Cam->FieldOfView);
        const float VFovRad = 2.f * FMath::Atan(FMath::Tan(HFovRad * 0.5f) / Aspect);

        const float Half = 0.5f * Spread;
        const float NeededArm = Half / FMath::Tan(VFovRad * 0.5f);

        const float NewArm = FMath::FInterpTo(SpringArm->TargetArmLength, NeededArm, DeltaSeconds, ZoomLerpSpeed);
        SpringArm->TargetArmLength = NewArm;
    }

    if (bDebugDraw && XY.Num() > 0)
    {
        const float Z = Avg3D.Z;
        for (const FVector2D& P : XY)
            DrawDebugPoint(GetWorld(), FVector(P.X, P.Y, Z), 12.f, FColor::Green, false, 0.f);
        DrawDebugLine(GetWorld(), FVector(A.X, A.Y, Z), FVector(B.X, B.Y, Z), FColor::Yellow, false, 0.f, 0, 2.f);
    }
}
