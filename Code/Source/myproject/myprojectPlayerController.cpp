#include "myprojectPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "myproject.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Kismet/GameplayStatics.h"
#include "Bouton.h"
#include "MyPlayerState.h"
#include "MyGameStateBase.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "RewindableComponent.h"
#include "RewindSubsystem.h"

void AmyprojectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(Logmyproject, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}
}

void AmyprojectPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AmyprojectPlayerController::Interact);

}

void AmyprojectPlayerController::Interact()
{
	// Client ? demande au serveur
	ServerInteract();
}

void AmyprojectPlayerController::ServerInteract_Implementation()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	// Vérifier proximité avec un bouton
	TArray<AActor*> Boutons;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABouton::StaticClass(), Boutons);

	for (AActor* Actor : Boutons)
	{
		ABouton* Btn = Cast<ABouton>(Actor);
		if (!Btn) continue;

		if (FVector::Dist(MyPawn->GetActorLocation(), Btn->GetActorLocation()) <= Btn->InteractionDistance)
		{
			AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
			if (PS)
			{
				Btn->Interact(PS); // Appel serveur
				break;
			}
		}
	}
}

float AmyprojectPlayerController::GetServerWorldTimeDelta() const
{
	return ServerWorldTimeDelta;
}

float AmyprojectPlayerController::GetServerWorldTime() const
{
	return GetWorld()->GetTimeSeconds() + ServerWorldTimeDelta;
}

void AmyprojectPlayerController::PostNetInit()
{
	Super::PostNetInit();

	if (GetLocalRole() != ROLE_Authority)
	{
		RequestWorldTime_Internal();

		if (NetworkClockUpdateFrequency > 0.f)
		{
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(
				TimerHandle,
				this,
				&AmyprojectPlayerController::RequestWorldTime_Internal,
				NetworkClockUpdateFrequency,
				true
			);
		}
	}
}

void AmyprojectPlayerController::RequestWorldTime_Internal()
{
	ServerRequestWorldTime(GetWorld()->GetTimeSeconds());
}

void AmyprojectPlayerController::ClientUpdateWorldTime_Implementation(float ClientTimestamp, float ServerTimestamp)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientTimestamp;
	RTTCircularBuffer.Add(RoundTripTime);

	float AdjustedRTT = 0.f;

	if (RTTCircularBuffer.Num() >= 10)
	{
		TArray<float> Tmp = RTTCircularBuffer;
		Tmp.Sort();

		// Moyenne des 8 valeurs médianes (ignore min et max)
		for (int32 i = 1; i < 9; ++i)
		{
			AdjustedRTT += Tmp[i];
		}
		AdjustedRTT /= 8.f;

		RTTCircularBuffer.RemoveAt(0);
	}
	else
	{
		AdjustedRTT = RoundTripTime;
	}

	ServerWorldTimeDelta = ServerTimestamp - ClientTimestamp - (AdjustedRTT / 2.f);
}

void AmyprojectPlayerController::ServerRequestWorldTime_Implementation(float ClientTimestamp)
{
	const float Timestamp = GetWorld()->GetTimeSeconds();
	ClientUpdateWorldTime(ClientTimestamp, Timestamp);
}