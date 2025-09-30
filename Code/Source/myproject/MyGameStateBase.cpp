#include "MyGameStateBase.h" 
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

	if (HasAuthority()) // Seulement côté serveur
	{
		// Choisir un nombre aléatoire de boutons
		MaxTaches = FMath::RandRange(3, 10); // exemple entre 3 et 10

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

	OnRep_Nbtache();
}

void AMyGameStateBase::OnRep_Nbtache()
{
	UE_LOG(LogTemp, Warning, TEXT("Nbtache répliqué = %d"), Nbtache);
}

void AMyGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyGameStateBase, Nbtache);
	DOREPLIFETIME(AMyGameStateBase, MaxTaches);
}
