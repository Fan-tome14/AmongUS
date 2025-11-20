#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyPlayerState.h"
#include "Bouton.h"
#include "myprojectGameMode.generated.h"

UCLASS()
class MYPROJECT_API AmyprojectGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AmyprojectGameMode();

    virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
    UPROPERTY(Transient)
    bool bHasMapChanged;

    
    UPROPERTY(EditAnywhere, Category = "Bouton")
    TSubclassOf<ABouton> BoutonClass;

public:
    void ChangeMap();
    void AssignRolesOnLevel();
    void SpawnBoutonsOnLevel();
    void ReturnToLobby();
    void CheckWinCondition();

private:
    UPROPERTY(EditDefaultsOnly, Category = "Game Flow")
    float GameDuration = 120.0f; // 2 minutes

    UPROPERTY(EditDefaultsOnly, Category = "Game Flow")
    float LobbyCountdownDuration = 30.0f; // 30 secondes
};
