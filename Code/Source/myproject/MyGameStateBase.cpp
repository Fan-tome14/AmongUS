#include "MyGameStateBase.h" 
#include "myprojectGameMode.h"
#include "Net/UnrealNetwork.h" 
#include "Engine/World.h" 
#include "Bouton.h"
#include "Kismet/GameplayStatics.h"

AMyGameStateBase::AMyGameStateBase()
{
	Nbtache = 0;
	MaxTaches = 0;
}

void AMyGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) 
	{
		// Choisir un nombre aléatoire de boutons
		MaxTaches = FMath::RandRange(3, 10); 

		// Spawn des boutons
		for (int32 i = 0; i < MaxTaches; i++)
		{
			FVector SpawnLocation;
			SpawnLocation.X = FMath::RandRange(-1000.f, 1000.f);
			SpawnLocation.Y = FMath::RandRange(-1000.f, 1000.f);
			SpawnLocation.Z = 0.f; // posé au sol

			FActorSpawnParameters Params;
			GetWorld()->SpawnActor<ABouton>(ABouton::StaticClass(), SpawnLocation, FRotator::ZeroRotator, Params);
		}

		// Initialiser Nbtache au nombre de boutons
		Nbtache = MaxTaches;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	// Plus de lancement automatique du timer ici
	if (World->GetMapName().EndsWith("Level"))
	{
		if (!GetWorldTimerManager().IsTimerActive(GameTimerHandle))
		{
			GetWorldTimerManager().SetTimer(
				GameTimerHandle,
				this,
				&AMyGameStateBase::GameCountdownTick,
				1.0f,
				true
			);
		}
	}
}

void AMyGameStateBase::ServerModifyNbtache_Implementation(AMyPlayerState* PlayerState)
{
	if (!PlayerState) return;

	EPlayerRole RoleValue = PlayerState->GetPlayerRole();

	switch (RoleValue)
	{
	case EPlayerRole::Gentil:
		Nbtache = FMath::Clamp(Nbtache - 1, 0, MaxTaches);
		UE_LOG(LogTemp, Warning, TEXT("Gentil a appuyé, Nbtache = %d"), Nbtache);
		break;

	case EPlayerRole::Mechant:
		Nbtache = FMath::Clamp(Nbtache + 1, 0, MaxTaches);
		UE_LOG(LogTemp, Warning, TEXT("Méchant a appuyé, Nbtache = %d"), Nbtache);
		break;

	case EPlayerRole::Mort:
		UE_LOG(LogTemp, Warning, TEXT("Mort ne peut pas interagir"));
		break;
	}

}


void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyGameStateBase, Nbtache);
	DOREPLIFETIME(AMyGameStateBase, MaxTaches);
}


void AMyGameStateBase::LobbyCountdownTick()
{
	if (PlayerArray.Num() < 2)
	{
		GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
		UE_LOG(LogTemp, Warning, TEXT("LobbyCountdown arrêté car moins de 2 joueurs."));
		return;
	}

	if (LobbyCountdown > 0)
	{
		LobbyCountdown--;
		UE_LOG(LogTemp, Warning, TEXT("LobbyCountdown = %d"), LobbyCountdown);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
		if (AmyprojectGameMode* GM = Cast<AmyprojectGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			GM->ChangeMap();
		}
	}
}

void AMyGameStateBase::GameCountdownTick()
{
	if (GameCountdown > 0)
	{
		GameCountdown--;
		UE_LOG(LogTemp, Warning, TEXT("GameCountdown = %d"), GameCountdown);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(GameTimerHandle);
		if (AmyprojectGameMode* GM = Cast<AmyprojectGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
		{
			GM->ReturnToLobby();
		}
	}
}

void AMyGameStateBase::StopGameCountdownTimer()
{
	GetWorldTimerManager().ClearTimer(GameTimerHandle);
}