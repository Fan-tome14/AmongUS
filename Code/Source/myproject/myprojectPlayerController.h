#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "myprojectPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class AMyPlayerState;

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

	/** RPC côté serveur pour interaction */
	UFUNCTION(Server, Reliable)
	void ServerInteract();


#pragma region NetworkClockSync

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Network Clock")
	float NetworkClockUpdateFrequency = 1.0f;

private:
	float ServerWorldTimeDelta = 0.f;
	TArray<float> RTTCircularBuffer;

public:
	UFUNCTION(BlueprintPure, Category = "Network Clock")
	float GetServerWorldTimeDelta() const;

	UFUNCTION(BlueprintPure, Category = "Network Clock")
	float GetServerWorldTime() const;

	virtual void PostNetInit() override;

private:
	void RequestWorldTime_Internal();

	UFUNCTION(Server, Unreliable)
	void ServerRequestWorldTime(float ClientTimestamp);

	UFUNCTION(Client, Unreliable)
	void ClientUpdateWorldTime(float ClientTimestamp, float ServerTimestamp);

#pragma endregion NetworkClockSync
};