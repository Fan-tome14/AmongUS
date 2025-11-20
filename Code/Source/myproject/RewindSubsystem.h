#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RewindSubsystem.generated.h"

class URewindableComponent;
class APlayerState;

USTRUCT()
struct FRewindState
{
    GENERATED_BODY()

    UPROPERTY()
    float ServerTime;

    UPROPERTY()
    FTransform Transform;

    FRewindState()
        : ServerTime(0.f)
        , Transform(FTransform::Identity)
    {
    }
};

// Historique par joueur
USTRUCT()
struct FComponentHistory
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FRewindState> History;
};

// Backup pour restauration
struct FComponentTransformBackup
{
    TWeakObjectPtr<URewindableComponent> Component;
    FTransform OriginalTransform;
};

UCLASS()
class MYPROJECT_API URewindSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    // N'existe que sur le serveur
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Tick pour enregistrer l'historique
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;

    // Enregistrement des components
    void RegisterRewindableComponent(URewindableComponent* Component);
    void UnregisterRewindableComponent(URewindableComponent* Component);

    // Validation du tir
    bool ValidateShot(
        float ShotServerTime,
        const FVector& TraceStart,
        const FRotator& TraceRotation,
        float TraceRange,
        APlayerState* TargetPlayerState,
        APlayerState* ShooterPlayerState,
        FHitResult& OutHitResult
    );

    // Configuration debug
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowDebugVisualization = true;

    UPROPERTY(EditAnywhere, Category = "Debug")
    FColor DebugCapsuleColor = FColor::Cyan;

    UPROPERTY(EditAnywhere, Category = "Debug")
    uint8 DebugCapsuleAlpha = 50;

    UPROPERTY(EditAnywhere, Category = "Debug")
    float DebugLineThickness = 1.0f;

private:
    // Durée maximale d'historique
    UPROPERTY()
    float MaxHistoryTime = 1.0f;

    // Components enregistrés (avec weak pointers pour sécurité)
    UPROPERTY()
    TArray<TWeakObjectPtr<URewindableComponent>> RegisteredComponents;

    // Historique par joueur
    UPROPERTY()
    TMap<APlayerState*, FComponentHistory> ComponentHistories;

    // Enregistrement des états
    void RecordRewindStates(float ServerTime);

    // Visualisation debug
    void DrawDebugHistory();

    // Récupération des états interpolés
    bool GetRewindStatesForTime(float Time, TMap<URewindableComponent*, FTransform>& OutTransforms);

    // Application et restauration des transformations
    void ApplyRewindTransforms(const TMap<URewindableComponent*, FTransform>& Transforms,
        TArray<FComponentTransformBackup>& OutBackups);
    void RestoreOriginalTransforms(const TArray<FComponentTransformBackup>& Backups);
};