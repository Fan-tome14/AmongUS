#pragma once 

#include "CoreMinimal.h" 
#include "GameFramework/GameStateBase.h" 
#include "MyPlayerState.h" 
#include "MyGameStateBase.generated.h" 

UCLASS()
class MYPROJECT_API AMyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMyGameStateBase();

	// Nombre actuel de tâches
	UPROPERTY(Replicated, BlueprintReadWrite)
	int32 Nbtache;

	// Nombre maximum de tâches (fixé au spawn des boutons)
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 MaxTaches;

	// Modifie Nbtache côté serveur 
	UFUNCTION(Server, Reliable)
	void ServerModifyNbtache(AMyPlayerState* PlayerState);


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadWrite)
	int32 LobbyCountdown;

	UPROPERTY(Replicated, BlueprintReadWrite)
	int32 GameCountdown;


	FTimerHandle LobbyTimerHandle;
	FTimerHandle GameTimerHandle;

	void LobbyCountdownTick();
	void GameCountdownTick();
	void StopGameCountdownTimer();

protected:
	virtual void BeginPlay() override;


};
