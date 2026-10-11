#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SniperGameMode.generated.h"

class ASniperEnemy;
class AStaticMeshActor;

/**
 * Runs the prototype mission: builds a test arena (floor and cover) at
 * runtime so any empty map works, spawns enemies, and tracks win/lose.
 */
UCLASS()
class SNIPERSHOOTER_API ASniperGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASniperGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;

	void NotifyEnemyKilled();
	void NotifyPlayerKilled();

	int32 GetKills() const { return Kills; }
	int32 GetTotalEnemies() const { return TotalEnemies; }
	bool IsMissionComplete() const { return bMissionComplete; }
	bool IsPlayerDead() const { return bPlayerDead; }
	bool IsGameOver() const { return bMissionComplete || bPlayerDead; }

	/** Build the floor and cover blocks. Turn off when using a hand-made map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	bool bBuildArena = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	TSubclassOf<ASniperEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 EnemyCount = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 CoverCount = 28;

	/** Enemies spawn between these distances (cm) from the player start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float SpawnRadiusMin = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float SpawnRadiusMax = 8000.f;

private:
	void BuildArena();
	void SpawnEnemies();
	AStaticMeshActor* SpawnBox(const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color);

	int32 Kills = 0;
	int32 TotalEnemies = 0;
	bool bMissionComplete = false;
	bool bPlayerDead = false;
};
