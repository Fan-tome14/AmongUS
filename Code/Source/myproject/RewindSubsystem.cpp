#include "RewindSubsystem.h"
#include "RewindableComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

bool URewindSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    if (const UWorld* World = Cast<UWorld>(Outer))
    {
        ENetMode Mode = World->GetNetMode();

        return (Mode != NM_Client);
    }
    return false;
}

TStatId URewindSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(URewindSubsystem, STATGROUP_Tickables);
}

void URewindSubsystem::RegisterRewindableComponent(URewindableComponent* Component)
{
    if (!Component)
        return;

    RegisteredComponents.AddUnique(Component);

    if (APawn* Pawn = Cast<APawn>(Component->GetOwner()))
    {
        if (APlayerState* PS = Pawn->GetPlayerState())
        {
            ComponentHistories.FindOrAdd(PS);
        }
    }
}

void URewindSubsystem::UnregisterRewindableComponent(URewindableComponent* Component)
{
    if (!Component)
        return;

    RegisteredComponents.Remove(Component);
}

void URewindSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UWorld* World = GetWorld();
    if (!World)
        return;

    float ServerTime = World->GetTimeSeconds();

    RecordRewindStates(ServerTime);

    if (bShowDebugVisualization)
    {
        DrawDebugHistory();
    }
}

void URewindSubsystem::RecordRewindStates(float ServerTime)
{
    // Enregistrer temps serveur, joueur et positions
    for (auto& Ptr : RegisteredComponents)
    {
        if (!Ptr.IsValid())
            continue;

        URewindableComponent* Comp = Ptr.Get();

        APawn* Pawn = Cast<APawn>(Comp->GetOwner());
        if (!Pawn)
            continue;

        APlayerState* PS = Pawn->GetPlayerState();
        if (!PS)
            continue;

        FRewindState State;
        State.ServerTime = ServerTime;
        State.Transform = Comp->GetComponentToWorld();

        FComponentHistory& History = ComponentHistories.FindOrAdd(PS);
        History.History.Add(State);

        while (History.History.Num() > 0 &&
            ServerTime - History.History[0].ServerTime > MaxHistoryTime)
        {
            History.History.RemoveAt(0);
        }
    }
}

void URewindSubsystem::DrawDebugHistory()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    float CurrentTime = World->GetTimeSeconds();

    for (auto& Elem : ComponentHistories)
    {
        APlayerState* PS = Elem.Key;
        FComponentHistory& History = Elem.Value;

        if (!PS || !PS->GetPawn())
            continue;

        URewindableComponent* Comp = PS->GetPawn()->FindComponentByClass<URewindableComponent>();
        if (!Comp)
            continue;

        for (const FRewindState& State : History.History)
        {
            float SnapshotAge = CurrentTime - State.ServerTime;
            float AgeRatio = FMath::Clamp(SnapshotAge / MaxHistoryTime, 0.0f, 1.0f);
            uint8 AlphaValue = static_cast<uint8>(DebugCapsuleAlpha * (1.0f - AgeRatio));

            FColor CapsuleColor = DebugCapsuleColor;
            CapsuleColor.A = AlphaValue;

            DrawDebugCapsule(
                World,
                State.Transform.GetLocation(),
                Comp->GetScaledCapsuleHalfHeight(),
                Comp->GetScaledCapsuleRadius(),
                State.Transform.GetRotation(),
                CapsuleColor,
                false,
                -1.0f,
                0,
                DebugLineThickness
            );
        }

        if (History.History.Num() > 0)
        {
            const FRewindState& LatestState = History.History.Last();
            FVector TextLocation = LatestState.Transform.GetLocation() +
                FVector(0, 0, Comp->GetScaledCapsuleHalfHeight() + 20.0f);

            DrawDebugString(
                World,
                TextLocation,
                PS->GetPlayerName(),
                nullptr,
                FColor::White,
                0.0f,
                true
            );
        }
    }
}

bool URewindSubsystem::GetRewindStatesForTime(float Time, TMap<URewindableComponent*, FTransform>& OutTransforms)
{
    for (auto& Elem : ComponentHistories)
    {
        APlayerState* PS = Elem.Key;
        FComponentHistory& History = Elem.Value;

        if (!PS || !PS->GetPawn())
            continue;

        URewindableComponent* Comp = PS->GetPawn()->FindComponentByClass<URewindableComponent>();
        if (!Comp)
            continue;

        if (History.History.Num() == 0)
            continue;

        int32 PrevIndex = -1;
        int32 NextIndex = -1;

        for (int32 i = 0; i < History.History.Num(); i++)
        {
            if (History.History[i].ServerTime <= Time)
                PrevIndex = i;
            else
            {
                NextIndex = i;
                break;
            }
        }

        if (PrevIndex == -1 && NextIndex == -1)
            continue;

        if (PrevIndex == -1)
        {
            OutTransforms.Add(Comp, History.History[NextIndex].Transform);
            continue;
        }

        if (NextIndex == -1)
        {
            OutTransforms.Add(Comp, History.History[PrevIndex].Transform);
            continue;
        }

        const FRewindState& PrevState = History.History[PrevIndex];
        const FRewindState& NextState = History.History[NextIndex];

        if (FMath::IsNearlyEqual(PrevState.ServerTime, NextState.ServerTime, 0.001f))
        {
            OutTransforms.Add(Comp, PrevState.Transform);
            continue;
        }

        float Alpha = (Time - PrevState.ServerTime) / (NextState.ServerTime - PrevState.ServerTime);
        Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

        FTransform InterpolatedTransform;
        InterpolatedTransform.SetLocation(
            FMath::Lerp(PrevState.Transform.GetLocation(), NextState.Transform.GetLocation(), Alpha)
        );
        InterpolatedTransform.SetRotation(
            FQuat::Slerp(PrevState.Transform.GetRotation(), NextState.Transform.GetRotation(), Alpha)
        );
        InterpolatedTransform.SetScale3D(FVector::OneVector);

        OutTransforms.Add(Comp, InterpolatedTransform);
    }

    return OutTransforms.Num() > 0;
}

void URewindSubsystem::ApplyRewindTransforms(
    const TMap<URewindableComponent*, FTransform>& Transforms,
    TArray<FComponentTransformBackup>& OutBackups)
{
    OutBackups.Empty();

    for (const auto& Elem : Transforms)
    {
        URewindableComponent* Comp = Elem.Key;
        if (!Comp || !Comp->IsValidLowLevel())
            continue;

        FComponentTransformBackup Backup;
        Backup.Component = Comp;
        Backup.OriginalTransform = Comp->GetComponentToWorld();
        OutBackups.Add(Backup);

        Comp->SetWorldTransform(Elem.Value, false, nullptr, ETeleportType::TeleportPhysics);
        Comp->UpdateComponentToWorld();

        if (bShowDebugVisualization)
        {
            DrawDebugCapsule(
                GetWorld(),
                Elem.Value.GetLocation(),
                Comp->GetScaledCapsuleHalfHeight(),
                Comp->GetScaledCapsuleRadius(),
                Elem.Value.GetRotation(),
                FColor::Magenta,
                false,
                3.0f,
                0,
                3.0f
            );
        }
    }
}

void URewindSubsystem::RestoreOriginalTransforms(const TArray<FComponentTransformBackup>& Backups)
{
    for (const FComponentTransformBackup& Backup : Backups)
    {
        if (Backup.Component.IsValid() && Backup.Component->IsValidLowLevel())
        {
            Backup.Component->SetWorldTransform(Backup.OriginalTransform, false, nullptr, ETeleportType::TeleportPhysics);
            Backup.Component->UpdateComponentToWorld();
        }
    }
}

bool URewindSubsystem::ValidateShot(
    float ShotServerTime,
    const FVector& TraceStart,
    const FRotator& TraceRotation,
    float TraceRange,
    APlayerState* TargetPlayerState,
    APlayerState* ShooterPlayerState,
    FHitResult& OutHitResult)
{
    UWorld* World = GetWorld();
    if (!World || !TargetPlayerState || !ShooterPlayerState)
        return false;

    float CurrentServerTime = World->GetTimeSeconds();
    float TimeDifference = FMath::Abs(CurrentServerTime - ShotServerTime);
    const float MaxAcceptableTimeDiff = 0.5f;

    if (TimeDifference > MaxAcceptableTimeDiff)
    {
        UE_LOG(LogTemp, Warning, TEXT("Shot rejected: Time difference too large (%.3fs)"), TimeDifference);
        return false;
    }

    TMap<URewindableComponent*, FTransform> RewindedTransforms;
    if (!GetRewindStatesForTime(ShotServerTime, RewindedTransforms))
    {
        UE_LOG(LogTemp, Warning, TEXT("Shot rejected: No rewind data for time %.3f"), ShotServerTime);
        return false;
    }

    TArray<FComponentTransformBackup> Backups;
    ApplyRewindTransforms(RewindedTransforms, Backups);

    FVector TraceEnd = TraceStart + (TraceRotation.Vector() * TraceRange);

    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;

    if (APawn* ShooterPawn = ShooterPlayerState->GetPawn())
        QueryParams.AddIgnoredActor(ShooterPawn);

    bool bHit = World->LineTraceSingleByChannel(
        OutHitResult,
        TraceStart,
        TraceEnd,
        ECC_Pawn,
        QueryParams
    );

    DrawDebugLine(
        World,
        TraceStart,
        bHit ? OutHitResult.Location : TraceEnd,
        FColor::Purple,
        false,
        5.0f,
        0,
        4.0f
    );

    if (bHit)
    {
        DrawDebugSphere(
            World,
            OutHitResult.Location,
            20.0f,
            12,
            FColor::Purple,
            false,
            5.0f
        );
    }

    RestoreOriginalTransforms(Backups);

    if (!bHit)
        return false;

    if (APawn* HitPawn = Cast<APawn>(OutHitResult.GetActor()))
    {
        if (APlayerState* HitPlayerState = HitPawn->GetPlayerState())
        {
            bool bIsCorrectTarget = (HitPlayerState == TargetPlayerState);

            if (bIsCorrectTarget)
            {
                UE_LOG(LogTemp, Warning, TEXT("Shot validated: %s hit %s"),
                    *ShooterPlayerState->GetPlayerName(),
                    *TargetPlayerState->GetPlayerName());
            }

            return bIsCorrectTarget;
        }
    }

    return false;
}