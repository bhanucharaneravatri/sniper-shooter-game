#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SniperEnemy.generated.h"

class UStaticMeshComponent;

/**
 * Simple enemy soldier. Patrols between two points, spots the player with a
 * view cone plus line-of-sight check, then stops and shoots (hitscan).
 */
UCLASS()
class SNIPERSHOOTER_API ASniperEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	ASniperEnemy();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	bool IsDead() const { return bIsDead; }

	/** Hits above (actor origin Z + this) count as headshots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float HeadshotHeightOffset = 55.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol")
	float PatrolSpeed = 150.f;

	/** Distance from the spawn point to each patrol waypoint. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Patrol")
	float PatrolRadius = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception")
	float SightRange = 7000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception")
	float SightHalfAngleDegrees = 65.f;

	/** After being shot, the enemy senses the player in any direction for this long. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Perception")
	float AlertDuration = 6.f;

	/** Time spent facing the last known position after losing sight. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float LoseInterestTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float ReactionTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float FireInterval = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Combat")
	float AttackDamage = 10.f;

protected:
	virtual void BeginPlay() override;

private:
	bool CanSeePlayer(const APawn& Player, float Now) const;
	void Attack(APawn& Player);
	void Patrol();
	void Die();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	TArray<FVector> PatrolPoints;
	int32 PatrolIndex = 0;

	float Health = 100.f;
	float AlertUntilTime = 0.f;
	float LastSeenTime = -100.f;
	float NextAttackTime = 0.f;
	bool bEngaged = false;
	bool bIsDead = false;
};
