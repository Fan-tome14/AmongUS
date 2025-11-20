#pragma once

#include "CoreMinimal.h"
#include "RewindableComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "myprojectCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);


UCLASS(abstract)
class AmyprojectCharacter : public ACharacter
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rewind", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<URewindableComponent> RewindCapsule;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, Category = "Input")
    UInputAction* MouseLookAction;

    /** Portée de tir */
    UPROPERTY(EditAnywhere, Category = "Shooting")
    float ShootRange = 10000.0f;

    /** Afficher les lignes de debug pour la prise de vue */
    UPROPERTY(EditAnywhere, Category = "Shooting")
    bool bShowShootDebug = true;

    /** Durée de la ligne de debug */
    UPROPERTY(EditAnywhere, Category = "Shooting")
    float DebugLineDuration = 2.0f;

public:
    AmyprojectCharacter();

protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void Move(const FInputActionValue& Value);

    void Look(const FInputActionValue& Value);

    /** Fonction de tire */
    void Fire();

    /** Server RPC */
    UFUNCTION(Server, Reliable)
    void ServerConfirmHit(
        float ClientTimestamp,
        const FVector& StartLocation,
        const FRotator& Rotation,
        APlayerState* TargetPlayerState
    );

public:
    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoMove(float Right, float Forward);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoLook(float Yaw, float Pitch);

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpStart();

    UFUNCTION(BlueprintCallable, Category = "Input")
    virtual void DoJumpEnd();

    FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

    FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};