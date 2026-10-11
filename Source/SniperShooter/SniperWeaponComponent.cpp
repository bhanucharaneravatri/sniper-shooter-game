#include "SniperWeaponComponent.h"
#include "SniperShooter.h"
#include "SniperEnemy.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

USniperWeaponComponent::USniperWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USniperWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	Camera = GetOwner()->FindComponentByClass<UCameraComponent>();
	CurrentAmmo = MagazineSize;
	ReserveAmmo = StartingReserveAmmo;
	BreathRemaining = BreathHoldDuration;
	CurrentFOV = HipFOV;

	if (Camera)
	{
		Camera->SetFieldOfView(CurrentFOV);
	}
	else
	{
		UE_LOG(LogSniper, Warning, TEXT("SniperWeaponComponent on %s found no CameraComponent."), *GetNameSafe(GetOwner()));
	}
}

void USniperWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Breath meter: holding steadies the scope until it runs out.
	const bool bWantsHold = bBreathKeyHeld && bIsScoped && BreathRemaining > 0.f && !bBreathExhausted;
	if (bWantsHold)
	{
		BreathRemaining = FMath::Max(0.f, BreathRemaining - DeltaTime);
		if (BreathRemaining <= 0.f)
		{
			bBreathExhausted = true;
		}
	}
	else
	{
		BreathRemaining = FMath::Min(BreathHoldDuration, BreathRemaining + DeltaTime * BreathRecoveryRate);
		if (BreathRemaining >= BreathHoldDuration * 0.4f)
		{
			bBreathExhausted = false;
		}
	}
	bHoldingBreath = bWantsHold;

	// Zoom.
	const float TargetFOV = bIsScoped ? ScopedFOV : HipFOV;
	CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ZoomInterpSpeed);
	if (Camera)
	{
		Camera->SetFieldOfView(CurrentFOV);
	}

	// Sway while scoped, and pull the view back down after recoil.
	float YawDelta = 0.f;
	float PitchDelta = 0.f;

	if (bIsScoped)
	{
		const float Time = GetWorld()->GetTimeSeconds();
		const ACharacter* Character = Cast<ACharacter>(GetOwner());
		float SwayScale = bHoldingBreath ? BreathHoldSwayMultiplier : 1.f;
		if (Character && Character->bIsCrouched)
		{
			SwayScale *= CrouchSwayMultiplier;
		}
		YawDelta += FMath::PerlinNoise1D(Time * 0.7f) * SwayDegreesPerSecond * SwayScale * DeltaTime;
		PitchDelta += FMath::PerlinNoise1D(Time * 0.9f + 31.4f) * SwayDegreesPerSecond * SwayScale * DeltaTime;
	}

	if (PendingRecoilRecovery > 0.f)
	{
		const float Step = FMath::Min(PendingRecoilRecovery, RecoilRecoverySpeed * DeltaTime);
		PitchDelta -= Step;
		PendingRecoilRecovery -= Step;
	}

	if (!FMath::IsNearlyZero(YawDelta) || !FMath::IsNearlyZero(PitchDelta))
	{
		ApplyViewDelta(YawDelta, PitchDelta);
	}
}

void USniperWeaponComponent::ApplyViewDelta(float YawDelta, float PitchDelta)
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;
	if (!Controller || !Controller->IsLocalPlayerController())
	{
		return;
	}

	FRotator Rotation = Controller->GetControlRotation();
	Rotation.Yaw += YawDelta;
	Rotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Rotation.Pitch + PitchDelta), -85.f, 85.f);
	Controller->SetControlRotation(Rotation);
}

bool USniperWeaponComponent::TryFire()
{
	UWorld* World = GetWorld();
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!World || !Pawn || !Camera || bIsReloading)
	{
		return false;
	}

	const float Now = World->GetTimeSeconds();
	if (Now < NextShotTime)
	{
		return false;
	}

	if (CurrentAmmo <= 0)
	{
		StartReload();
		return false;
	}

	--CurrentAmmo;
	NextShotTime = Now + ShotInterval;

	// Aim from the camera along the control rotation, with a small spread cone.
	const FVector Start = Camera->GetComponentLocation();
	const float SpreadDegrees = bIsScoped ? (bHoldingBreath ? 0.02f : ScopedSpreadDegrees) : HipSpreadDegrees;
	const FVector Direction = FMath::VRandCone(Pawn->GetControlRotation().Vector(), FMath::DegreesToRadians(SpreadDegrees));
	const FVector End = Start + Direction * Range;

	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SniperShot), true, GetOwner());
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
	const FVector Impact = bHit ? Hit.ImpactPoint : End;

	// Tracer and impact marker (debug draw; replace with Niagara later).
	const FVector TracerStart = Start + Direction * 80.f + FVector(0.f, 0.f, -10.f);
	DrawDebugLine(World, TracerStart, Impact, FColor::Yellow, false, 0.12f, 0, 1.5f);

	if (bHit)
	{
		DrawDebugSphere(World, Impact, 12.f, 8, FColor::Orange, false, 2.f, 0, 1.f);

		if (ASniperEnemy* Enemy = Cast<ASniperEnemy>(Hit.GetActor()))
		{
			const bool bHeadshot = Impact.Z > Enemy->GetActorLocation().Z + Enemy->HeadshotHeightOffset;
			const float FinalDamage = Damage * (bHeadshot ? HeadshotMultiplier : 1.f);

			UGameplayStatics::ApplyPointDamage(Enemy, FinalDamage, Direction, Hit, Pawn->GetController(), GetOwner(), UDamageType::StaticClass());

			LastHitTime = Now;
			bLastHitHeadshot = bHeadshot;
			bLastHitKill = Enemy->IsDead();
		}
	}

	// Recoil kick, partially recovered over the next moments.
	const float Kick = RecoilPitchDegrees;
	ApplyViewDelta(FMath::FRandRange(-RecoilYawDegrees, RecoilYawDegrees), Kick);
	PendingRecoilRecovery += Kick * RecoilRecoveryFraction;

	OnWeaponFired.Broadcast();
	return true;
}

void USniperWeaponComponent::StartReload()
{
	if (bIsReloading || CurrentAmmo >= MagazineSize || ReserveAmmo <= 0)
	{
		return;
	}

	bIsReloading = true;
	GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &USniperWeaponComponent::FinishReload, ReloadTime, false);
}

void USniperWeaponComponent::FinishReload()
{
	const int32 Needed = MagazineSize - CurrentAmmo;
	const int32 Taken = FMath::Min(Needed, ReserveAmmo);
	CurrentAmmo += Taken;
	ReserveAmmo -= Taken;
	bIsReloading = false;
}

float USniperWeaponComponent::GetScopeAlpha() const
{
	const float Span = FMath::Max(HipFOV - ScopedFOV, KINDA_SMALL_NUMBER);
	return FMath::Clamp((HipFOV - CurrentFOV) / Span, 0.f, 1.f);
}

float USniperWeaponComponent::GetLookSensitivityScale() const
{
	return FMath::Clamp(CurrentFOV / FMath::Max(HipFOV, 1.f), 0.1f, 1.f);
}

float USniperWeaponComponent::GetBreathFraction() const
{
	return FMath::Clamp(BreathRemaining / FMath::Max(BreathHoldDuration, KINDA_SMALL_NUMBER), 0.f, 1.f);
}
