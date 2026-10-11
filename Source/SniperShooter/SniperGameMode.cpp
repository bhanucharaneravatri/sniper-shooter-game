#include "SniperGameMode.h"
#include "SniperShooter.h"
#include "SniperCharacter.h"
#include "SniperEnemy.h"
#include "SniperHUD.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Materials/MaterialInstanceDynamic.h"

ASniperGameMode::ASniperGameMode()
{
	DefaultPawnClass = ASniperCharacter::StaticClass();
	HUDClass = ASniperHUD::StaticClass();
	EnemyClass = ASniperEnemy::StaticClass();
}

void ASniperGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Make sure the player has somewhere to spawn, even in an empty map.
	bool bHasPlayerStart = false;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		bHasPlayerStart = true;
		break;
	}
	if (!bHasPlayerStart)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		World->SpawnActor<APlayerStart>(FVector(0.f, 0.f, 120.f), FRotator::ZeroRotator, Params);
	}

	if (bBuildArena)
	{
		BuildArena();
	}
}

void ASniperGameMode::StartPlay()
{
	Super::StartPlay();
	SpawnEnemies();
}

AStaticMeshActor* ASniperGameMode::SpawnBox(const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color)
{
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (!Cube || !GetWorld())
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* Actor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation, Params);
	if (!Actor)
	{
		return nullptr;
	}

	Actor->SetMobility(EComponentMobility::Movable);
	UStaticMeshComponent* Mesh = Actor->GetStaticMeshComponent();
	Mesh->SetStaticMesh(Cube);
	Actor->SetActorScale3D(Scale);

	if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		Material->SetVectorParameterValue(TEXT("Color"), Color);
	}
	return Actor;
}

void ASniperGameMode::BuildArena()
{
	// Floor: a 300 m square slab whose top face sits at Z = 0 (the cube is 100 cm).
	SpawnBox(FVector(0.f, 0.f, -50.f), FRotator::ZeroRotator, FVector(300.f, 300.f, 1.f), FLinearColor(0.12f, 0.18f, 0.08f));

	// Cover: random blocks scattered around, kept clear of the spawn point.
	for (int32 i = 0; i < CoverCount; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(1200.f, 9000.f);
		const FVector Scale(FMath::FRandRange(2.f, 6.f), FMath::FRandRange(0.5f, 1.5f), FMath::FRandRange(1.5f, 3.5f));
		const FVector Location(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, Scale.Z * 50.f);
		const FRotator Rotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);
		const float Shade = FMath::FRandRange(0.25f, 0.45f);
		SpawnBox(Location, Rotation, Scale, FLinearColor(Shade, Shade, Shade * 0.9f));
	}
}

void ASniperGameMode::SpawnEnemies()
{
	UWorld* World = GetWorld();
	UClass* ClassToSpawn = EnemyClass ? EnemyClass.Get() : ASniperEnemy::StaticClass();
	if (!World)
	{
		return;
	}

	TotalEnemies = 0;
	for (int32 i = 0; i < EnemyCount; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 2.f * PI);
		const float Radius = FMath::FRandRange(SpawnRadiusMin, SpawnRadiusMax);
		const FVector Location(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 110.f);
		const FRotator Rotation(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (World->SpawnActor<ASniperEnemy>(ClassToSpawn, Location, Rotation, Params))
		{
			++TotalEnemies;
		}
	}

	UE_LOG(LogSniper, Log, TEXT("Mission started: %d enemies."), TotalEnemies);
}

void ASniperGameMode::NotifyEnemyKilled()
{
	++Kills;
	if (TotalEnemies > 0 && Kills >= TotalEnemies && !bPlayerDead)
	{
		bMissionComplete = true;
		UE_LOG(LogSniper, Log, TEXT("Mission complete."));
	}
}

void ASniperGameMode::NotifyPlayerKilled()
{
	bPlayerDead = true;
	UE_LOG(LogSniper, Log, TEXT("Player killed."));
}
