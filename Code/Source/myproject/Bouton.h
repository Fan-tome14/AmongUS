#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyPlayerState.h"
#include "Bouton.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class MYPROJECT_API ABouton : public AActor
{
    GENERATED_BODY()

public:
    ABouton();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Zone de collision pour détecter les joueurs proches
    UPROPERTY(VisibleAnywhere)
    USphereComponent* SphereComp;

    // Mesh du bouton (cube)
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MeshComp;

    // Distance maximale pour interaction
    UPROPERTY(EditAnywhere)
    float InteractionDistance;

    // Interaction réelle (appelée côté serveur)
    void Interact(AMyPlayerState* PlayerState);
};
