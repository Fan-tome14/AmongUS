#include "myprojectGameMode.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AmyprojectGameMode::AmyprojectGameMode()
{
    bHasMapChanged = false;
}

void AmyprojectGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    UWorld* World = GetWorld();
    if (!World) return;

    FString CurrentMapName = World->GetMapName();
    CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix); 

    
    if (CurrentMapName.Equals("Lobby"))
    {
        AGameStateBase* GS = GetGameState<AGameStateBase>();
        if (!GS) return;

        int32 PlayerCount = GS->PlayerArray.Num();
        UE_LOG(LogTemp, Warning, TEXT("Nombre de joueurs connectés : %d"), PlayerCount);

        if (!bHasMapChanged && PlayerCount >= 4)
        {
            bHasMapChanged = true;
            UE_LOG(LogTemp, Warning, TEXT("4 joueurs connectés ! La map changera dans 3 secondes"));

            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, this, &AmyprojectGameMode::ChangeMap, 3.0f, false);
        }
    }
    
    else if (CurrentMapName.Equals("Level"))
    {
        if (!HasAuthority()) return;

        
        AssignRolesOnLevel();


    }
}

void AmyprojectGameMode::ChangeMap()
{
    UWorld* World = GetWorld();
    if (World && World->GetNetMode() == NM_ListenServer)
    {
        FString MapPath = "/Game/Level";
        UE_LOG(LogTemp, Warning, TEXT("ServerTravel vers %s"), *MapPath);

        World->ServerTravel(MapPath + "?listen");

        
    }

    SpawnBoutonsOnLevel();

}

void AmyprojectGameMode::AssignRolesOnLevel()
{
    AGameStateBase* GS = GetGameState<AGameStateBase>();
    if (!GS) return;

    TArray<AMyPlayerState*> Players;

    for (APlayerState* PS : GS->PlayerArray)
    {
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
        if (MyPS)
            Players.Add(MyPS);
    }

    if (Players.Num() == 0) return;

    
    for (int32 i = 0; i < Players.Num(); i++)
    {
        int32 SwapIndex = FMath::RandRange(0, Players.Num() - 1);
        Players.Swap(i, SwapIndex);
    }

    
    Players[0]->SetPlayerRole(EPlayerRole::Gentil);

    
    for (int32 i = 1; i < Players.Num(); i++)
    {
        EPlayerRole NewRole = (FMath::RandBool()) ? EPlayerRole::Mechant : EPlayerRole::Mort;
        Players[i]->SetPlayerRole(NewRole);
    }

    
    for (AMyPlayerState* PS : Players)
    {
        const TCHAR* RoleText = (PS->GetPlayerRole() == EPlayerRole::Gentil) ? TEXT("Gentil") :
            (PS->GetPlayerRole() == EPlayerRole::Mechant) ? TEXT("Méchant") : TEXT("Mort");
        UE_LOG(LogTemp, Warning, TEXT("Joueur %s est %s"), *PS->GetPlayerName(), RoleText);
    }

    
}

void AmyprojectGameMode::SpawnBoutonsOnLevel()
{
    if (!HasAuthority() || !BoutonClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FString CurrentMapName = World->GetMapName();
    CurrentMapName.RemoveFromStart(World->StreamingLevelsPrefix);
    if (!CurrentMapName.Equals("Level"))
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnBoutons annulé : pas sur Level"));
        return;
    }

    for (int32 i = 0; i < 5; i++)
    {
        FVector SpawnLocation;
        SpawnLocation.X = FMath::RandRange(-1000.f, 1000.f);
        SpawnLocation.Z = FMath::RandRange(-1000.f, 1000.f);
        SpawnLocation.Y = 0.f; 

        FRotator SpawnRotation = FRotator::ZeroRotator;

        ABouton* NewBouton = World->SpawnActor<ABouton>(BoutonClass, SpawnLocation, SpawnRotation);
        if (NewBouton)
        {
            
            UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/bouton"));
            if (Mat && NewBouton->MeshComp)
            {
                NewBouton->MeshComp->SetMaterial(0, Mat);
            }
        }
    }
}
