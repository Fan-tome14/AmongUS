#include "RewindableComponent.h"
#include "RewindSubsystem.h"
#include "Engine/World.h"

URewindableComponent::URewindableComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    SetHiddenInGame(false);
    SetVisibility(true);

    SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SetCollisionObjectType(ECC_Pawn);
    SetCollisionResponseToAllChannels(ECR_Ignore);
    SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
}

void URewindableComponent::BeginPlay()
{
    Super::BeginPlay();

    // Enregistrement uniquement côté serveur
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (UWorld* World = GetWorld())
    {
        if (URewindSubsystem* RewindSubsystem = World->GetSubsystem<URewindSubsystem>())
        {
            RewindSubsystem->RegisterRewindableComponent(this);
        }
    }
}

void URewindableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Désenregistrement uniquement côté serveur
    if (GetOwner() && GetOwner()->HasAuthority())
    {
        if (UWorld* World = GetWorld())
        {
            if (URewindSubsystem* RewindSubsystem = World->GetSubsystem<URewindSubsystem>())
            {
                RewindSubsystem->UnregisterRewindableComponent(this);
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}