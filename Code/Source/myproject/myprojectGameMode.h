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

private:
    void ChangeMap();
    void AssignRolesOnLevel();
    void SpawnBoutonsOnLevel();
};
