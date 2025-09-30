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

void AmyprojectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
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

	// Mapping contexts
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

	// Bind touche P
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
