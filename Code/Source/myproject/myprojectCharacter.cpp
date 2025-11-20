#include "myprojectCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "myproject.h"
#include "RewindableComponent.h"
#include "RewindSubsystem.h"
#include "MyPlayerState.h"
#include "myprojectPlayerController.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AmyprojectCharacter::AmyprojectCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 500.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Creation de RewindCapsule
    RewindCapsule = CreateDefaultSubobject<URewindableComponent>(TEXT("RewindCapsule"));
    RewindCapsule->SetupAttachment(GetCapsuleComponent());
    RewindCapsule->SetCapsuleRadius(GetCapsuleComponent()->GetScaledCapsuleRadius() * 1.15f);
    RewindCapsule->SetCapsuleHalfHeight(GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 1.15f);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void AmyprojectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AmyprojectCharacter::Move);
        EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AmyprojectCharacter::Look);

        // Looking
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AmyprojectCharacter::Look);

        // Shooting
        PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &AmyprojectCharacter::Fire);
        PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AmyprojectCharacter::Fire);
    }
}

void AmyprojectCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    DoMove(MovementVector.X, MovementVector.Y);
}

void AmyprojectCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AmyprojectCharacter::DoMove(float Right, float Forward)
{
    if (GetController() != nullptr)
    {
        const FRotator Rotation = GetController()->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
 
        AddMovementInput(ForwardDirection, Forward);
        AddMovementInput(RightDirection, Right);
    }
}

void AmyprojectCharacter::DoLook(float Yaw, float Pitch)
{
    if (GetController() != nullptr)
    {
        AddControllerYawInput(Yaw);
        AddControllerPitchInput(Pitch);
    }
}

void AmyprojectCharacter::DoJumpStart()
{
    Jump();
}

void AmyprojectCharacter::DoJumpEnd()
{
    StopJumping();
}

void AmyprojectCharacter::Fire()
{
    UWorld* World = GetWorld();
    if (!World || !IsLocallyControlled())
        return;

    if (!FollowCamera)
        return;

    FVector StartLocation = FollowCamera->GetComponentLocation();
    FRotator CameraRotation = FollowCamera->GetComponentRotation();
    FVector EndLocation = StartLocation + (CameraRotation.Vector() * ShootRange);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    Params.bTraceComplex = false;

    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Pawn,
        Params
    );

    if (bShowShootDebug)
    {
        FColor DebugColor = bHit ? FColor::Red : FColor::Green;
        DrawDebugLine(
            World,
            StartLocation,
            bHit ? HitResult.Location : EndLocation,
            DebugColor,
            false,
            DebugLineDuration,
            0,
            2.0f
        );

        if (bHit)
        {
            DrawDebugSphere(
                World,
                HitResult.Location,
                10.0f,
                12,
                FColor::Red,
                false,
                DebugLineDuration
            );
        }
    }

    // Vérifier si on a touché un joueur
    AMyPlayerState* TargetPS = nullptr;
    if (bHit)
    {
        if (ACharacter* HitCharacter = Cast<ACharacter>(HitResult.GetActor()))
        {
            TargetPS = HitCharacter->GetPlayerState<AMyPlayerState>();
        }
    }

    if (TargetPS)
    {
        // Envoyer RPC avec toutes les informations
        AmyprojectPlayerController* PC = Cast<AmyprojectPlayerController>(GetController());
        float EstimatedServerTime = PC ? PC->GetServerWorldTime() : World->GetTimeSeconds();

        ServerConfirmHit(EstimatedServerTime, StartLocation, CameraRotation, TargetPS);

        UE_LOG(LogTemplateCharacter, Log, TEXT("Client Fire: Hit %s @ %.3f"),
            *TargetPS->GetPlayerName(), EstimatedServerTime);
    }
}

void AmyprojectCharacter::ServerConfirmHit_Implementation(
    float ClientTimestamp,
    const FVector& StartLocation,
    const FRotator& Rotation,
    APlayerState* TargetPlayerState)
{
    UWorld* World = GetWorld();
    if (!World || !TargetPlayerState)
        return;

    AMyPlayerState* ShooterPS = GetPlayerState<AMyPlayerState>();
    if (!ShooterPS)
        return;

    // Demander au subsystème de valider le tir
    if (URewindSubsystem* RewindSubsystem = World->GetSubsystem<URewindSubsystem>())
    {
        FHitResult ValidationResult;

        bool bShotValid = RewindSubsystem->ValidateShot(
            ClientTimestamp,
            StartLocation,
            Rotation,
            ShootRange,
            TargetPlayerState,
            ShooterPS,
            ValidationResult
        );

        if (bShotValid)
        {
            UE_LOG(LogTemplateCharacter, Warning, TEXT(" SHOT CONFIRMED: %s hit %s"),
                *ShooterPS->GetPlayerName(),
                *TargetPlayerState->GetPlayerName());

        }
    }
}