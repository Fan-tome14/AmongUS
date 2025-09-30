#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
    Gentil UMETA(DisplayName = "Gentil"),
    Mechant UMETA(DisplayName = "Méchant"),
    Mort UMETA(DisplayName = "Mort")
};

UCLASS()
class MYPROJECT_API AMyPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AMyPlayerState();

protected:
    UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Role")
    EPlayerRole PlayerRole;

public:
    UFUNCTION(BlueprintCallable, Category = "Role")
    void SetPlayerRole(EPlayerRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Role")
    EPlayerRole GetPlayerRole() const;

    UFUNCTION()
    void OnRep_PlayerRole();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
