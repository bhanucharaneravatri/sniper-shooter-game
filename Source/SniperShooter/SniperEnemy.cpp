#include "SniperEnemy.h"
#include "SniperShooter.h"
#include "SniperCharacter.h"
#include "SniperGameMode.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ASniperEnemy::ASniperEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// A plain AIController is enough to let the character movement component run.
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();

	GetCapsuleComponent()->InitCapsuleSize(35.f, 90.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 180.f, 0.f);
	Movement->MaxWalkSpeed = PatrolSpeed;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	// Placeholder soldier: a cylinder body with a sphere head. Hits are
	// detected on the capsule, so the visual meshes have no collision.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -15.f));
	BodyMesh->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.5f));

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetCapsuleComponent());
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 75.f));
	HeadMesh->SetRelativeScale3D(FVector(0.4f));

	if (CylinderFinder.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderFinder.Object);
	}
	if (SphereFinder.Succeeded())
	{
		HeadMesh->SetStaticMesh(SphereFinder.Object);
	}
}

void ASniperEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	// Two waypoints on opposite sides of the spawn point, along a random axis.
	const FVector Origin = GetActorLocation();
	const FVector Axis = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector();
	PatrolPoints.Add(Origin + Axis * PatrolRadius);
	PatrolPoints.Add(Origin - Axis * PatrolRadius);

	// Tint the placeholder meshes (BasicShapeMaterial exposes a "Color" parameter).
	if (UMaterialInstanceDynamic* BodyMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		BodyMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.45f, 0.08f, 0.06f));
	}
	if (UMaterialInstanceDynamic* HeadMaterial = HeadMesh->CreateAndSetMaterialInstanceDynamic(0))
	{
		HeadMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.8f, 0.6f, 0.45f));
	}
}

void ASniperEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	const ASniperCharacter* PlayerCharacter = Cast<ASniperCharacter>(Player);
	const bool bPlayerAlive = PlayerCharacter && !PlayerCharacter->IsDead();

	const bool bSeesPlayer = bPlayerAlive && CanSeePlayer(*Player, Now);
	if (bSeesPlayer)
	{
		if (!bEngaged)
		{
			bEngaged = true;
			NextAttackTime = Now + ReactionTime;
		}
		LastSeenTime = Now;
	}

	const bool bInCombat = bPlayerAlive && bEngaged && (Now - LastSeenTime) < LoseInterestTime;
	if (bInCombat)
	{
		// Stand still, turn toward the player and shoot when able.
		const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
		const FRotator DesiredRotation(0.f, ToPlayer.Rotation().Yaw, 0.f);
		SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), DesiredRotation, DeltaSeconds, 240.f));

		if (bSeesPlayer && Now >= NextAttackTime)
		{
			Attack(*Player);
			NextAttackTime = Now + FireInterval * FMath::FRandRange(0.8f, 1.4f);
		}
	}
	else
	{
		bEngaged = false;
		Patrol();
	}
}

bool ASniperEnemy::CanSeePlayer(const APawn& Player, float Now) const
{
	const FVector EyeLocation = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector TargetLocation = Player.GetActorLocation() + FVector(0.f, 0.f, 40.f);
	const FVector ToPlayer = TargetLocation - EyeLocation;

	if (ToPlayer.Size() > SightRange)
	{
		return false;
	}

	// Outside the view cone only works while alerted by a recent hit.
	if (Now >= AlertUntilTime)
	{
		const float Dot = FVector::DotProduct(GetActorForwardVector(), ToPlayer.GetSafeNormal2D());
		if (Dot < FMath::Cos(FMath::DegreesToRadians(SightHalfAngleDegrees)))
		{
			return false;
		}
	}

	// Anything between the eye and the player blocks the view.
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(EnemySight), false, this);
	Params.AddIgnoredActor(&Player);
	return !GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TargetLocation, ECC_Visibility, Params);
}

void ASniperEnemy::Attack(APawn& Player)
{
	const FVector Eye = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector Target = Player.GetActorLocation();
	const float Distance = FVector::Dist(Eye, Target);

	// Accuracy falls off with distance.
	const float HitChance = FMath::Clamp(0.65f - Distance / 12000.f, 0.1f, 0.65f);
	const bool bHit = FMath::FRand() < HitChance;
	const FVector End = bHit ? Target : Target + FMath::VRand() * 200.f;

	DrawDebugLine(GetWorld(), Eye, End, FColor::Red, false, 0.15f, 0, 2.f);

	if (bHit)
	{
		UGameplayStatics::ApplyDamage(&Player, AttackDamage, GetController(), this, UDamageType::StaticClass());
	}
}

void ASniperEnemy::Patrol()
{
	if (PatrolPoints.Num() == 0)
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	FVector ToTarget = PatrolPoints[PatrolIndex] - GetActorLocation();
	ToTarget.Z = 0.f;

	if (ToTarget.SizeSquared() < FMath::Square(100.f))
	{
		PatrolIndex = (PatrolIndex + 1) % PatrolPoints.Num();
		return;
	}

	AddMovementInput(ToTarget.GetSafeNormal(), 1.f);
}

float ASniperEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return 0.f;
	}

	const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health -= Actual;
	AlertUntilTime = GetWorld()->GetTimeSeconds() + AlertDuration;

	if (Health <= 0.f)
	{
		Die();
	}
	return Actual;
}

void ASniperEnemy::Die()
{
	bIsDead = true;

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Topple the placeholder body onto its back (positive pitch sends the top toward -X).
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, -60.f));
	BodyMesh->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	HeadMesh->SetRelativeLocation(FVector(-95.f, 0.f, -65.f));

	if (ASniperGameMode* GameMode = GetWorld()->GetAuthGameMode<ASniperGameMode>())
	{
		GameMode->NotifyEnemyKilled();
	}

	DetachFromControllerPendingDestroy();
	SetLifeSpan(5.f);
}
