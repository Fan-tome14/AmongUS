#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "myprojectPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

UCLASS()
class MYPROJECT_API AmyprojectPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/** Action d'interaction (touche P) */
	void Interact();

	/** RPC côté serveur */
	UFUNCTION(Server, Reliable)
	void ServerInteract();
};
