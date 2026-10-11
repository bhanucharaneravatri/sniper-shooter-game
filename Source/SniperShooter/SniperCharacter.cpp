#include "SniperCharacter.h"
#include "SniperShooter.h"
#include "SniperGameMode.h"
#include "SniperWeaponComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASniperCharacter::ASniperCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.f, 88.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->NavAgentProps.bCanCrouch = true;
	Movement->MaxWalkSpeed = WalkSpeed;
	Movement->MaxWalkSpeedCrouched = 200.f;
	Movement->JumpZVelocity = 420.f;
	Movement->AirControl = 0.2f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 60.f));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(90.f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));

	// Placeholder rifle: a long box with a smaller box on top as the scope.
	RifleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RifleMesh"));
	RifleMesh->SetupAttachment(Camera);
	RifleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RifleMesh->SetCastShadow(false);
	RifleMesh->SetRelativeLocation(FVector(45.f, 16.f, -14.f));
	RifleMesh->SetRelativeScale3D(FVector(0.9f, 0.05f, 0.07f));

	ScopeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScopeMesh"));
	ScopeMesh->SetupAttachment(Camera);
	ScopeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ScopeMesh->SetCastShadow(false);
	ScopeMesh->SetRelativeLocation(FVector(40.f, 16.f, -6.f));
	ScopeMesh->SetRelativeScale3D(FVector(0.28f, 0.05f, 0.05f));

	if (CubeFinder.Succeeded())
	{
		RifleMesh->SetStaticMesh(CubeFinder.Object);
		ScopeMesh->SetStaticMesh(CubeFinder.Object);
	}

	MuzzleFlash = CreateDefaultSubobject<UPointLightComponent>(TEXT("MuzzleFlash"));
	MuzzleFlash->SetupAttachment(Camera);
	MuzzleFlash->SetRelativeLocation(FVector(130.f, 16.f, -14.f));
	MuzzleFlash->SetIntensity(8000.f);
	MuzzleFlash->SetLightColor(FLinearColor(1.f, 0.65f, 0.25f));
	MuzzleFlash->SetAttenuationRadius(800.f);
	MuzzleFlash->SetCastShadows(false);
	MuzzleFlash->SetVisibility(false);

	Weapon = CreateDefaultSubobject<USniperWeaponComponent>(TEXT("Weapon"));
}

void ASniperCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;
	Weapon->OnWeaponFired.AddUObject(this, &ASniperCharacter::HandleWeaponFired);
}

void ASniperCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bScoped = Weapon->IsScoped();
	Weapon->SetBreathKeyHeld(bSprintHeld && !bDead);

	// Shift sprints normally and holds your breath while scoped.
	float TargetSpeed = bScoped ? ScopedWalkSpeed : (bSprintHeld ? SprintSpeed : WalkSpeed);
	if (bIsCrouched)
	{
		TargetSpeed = FMath::Min(TargetSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
	}
	GetCharacterMovement()->MaxWalkSpeed = TargetSpeed;

	// The rifle is hidden while looking through the scope.
	const bool bShowRifle = Weapon->GetScopeAlpha() < 0.5f && !bDead;
	RifleMesh->SetVisibility(bShowRifle);
	ScopeMesh->SetVisibility(bShowRifle);
}

void ASniperCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASniperCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASniperCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ASniperCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASniperCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASniperCharacter::JumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASniperCharacter::JumpReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASniperCharacter::FirePressed);
	PlayerInputComponent->BindAction("Scope", IE_Pressed, this, &ASniperCharacter::ScopePressed);
	PlayerInputComponent->BindAction("Scope", IE_Released, this, &ASniperCharacter::ScopeReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASniperCharacter::ReloadPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASniperCharacter::SprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASniperCharacter::SprintReleased);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASniperCharacter::CrouchPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASniperCharacter::CrouchReleased);
	PlayerInputComponent->BindAction("Restart", IE_Pressed, this, &ASniperCharacter::RestartPressed);
}

void ASniperCharacter::MoveForward(float Value)
{
	if (!bDead && Value != 0.f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ASniperCharacter::MoveRight(float Value)
{
	if (!bDead && Value != 0.f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void ASniperCharacter::Turn(float Value)
{
	if (!bDead)
	{
		AddControllerYawInput(Value * Weapon->GetLookSensitivityScale());
	}
}

void ASniperCharacter::LookUp(float Value)
{
	if (!bDead)
	{
		AddControllerPitchInput(Value * Weapon->GetLookSensitivityScale());
	}
}

void ASniperCharacter::JumpPressed()
{
	if (!bDead)
	{
		Jump();
	}
}

void ASniperCharacter::JumpReleased()
{
	StopJumping();
}

void ASniperCharacter::FirePressed()
{
	if (!bDead)
	{
		Weapon->TryFire();
	}
}

void ASniperCharacter::ScopePressed()
{
	if (!bDead)
	{
		Weapon->SetScoped(true);
	}
}

void ASniperCharacter::ScopeReleased()
{
	Weapon->SetScoped(false);
}

void ASniperCharacter::ReloadPressed()
{
	if (!bDead)
	{
		Weapon->StartReload();
	}
}

void ASniperCharacter::SprintPressed()
{
	bSprintHeld = true;
}

void ASniperCharacter::SprintReleased()
{
	bSprintHeld = false;
}

void ASniperCharacter::CrouchPressed()
{
	if (!bDead)
	{
		Crouch();
	}
}

void ASniperCharacter::CrouchReleased()
{
	UnCrouch();
}

void ASniperCharacter::RestartPressed()
{
	const ASniperGameMode* GameMode = GetWorld()->GetAuthGameMode<ASniperGameMode>();
	if (GameMode && GameMode->IsGameOver())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ConsoleCommand(TEXT("RestartLevel"));
		}
	}
}

void ASniperCharacter::HandleWeaponFired()
{
	MuzzleFlash->SetVisibility(true);
	GetWorldTimerManager().SetTimer(MuzzleFlashTimer, this, &ASniperCharacter::HideMuzzleFlash, 0.05f, false);
}

void ASniperCharacter::HideMuzzleFlash()
{
	MuzzleFlash->SetVisibility(false);
}

float ASniperCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bDead)
	{
		return 0.f;
	}

	const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Max(0.f, Health - Actual);

	if (Health <= 0.f)
	{
		Die();
	}
	return Actual;
}

void ASniperCharacter::Die()
{
	bDead = true;
	Weapon->SetScoped(false);
	GetCharacterMovement()->DisableMovement();

	if (ASniperGameMode* GameMode = GetWorld()->GetAuthGameMode<ASniperGameMode>())
	{
		GameMode->NotifyPlayerKilled();
	}
}
