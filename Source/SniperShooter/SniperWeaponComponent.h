#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "SniperWeaponComponent.generated.h"

class UCameraComponent;

DECLARE_MULTICAST_DELEGATE(FOnWeaponFired);

/**
 * Bolt-action sniper rifle logic: scope zoom, sway, breath hold, recoil,
 * ammo, reload and hitscan firing. Attach to a pawn that has a UCameraComponent.
 */
UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent))
class SNIPERSHOOTER_API USniperWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USniperWeaponComponent();

	// ---- Tuning ----------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 MagazineSize = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Ammo")
	int32 StartingReserveAmmo = 30;

	/** Seconds between shots (bolt cycle). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float ShotInterval = 1.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float ReloadTime = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float Damage = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float HeadshotMultiplier = 2.f;

	/** Trace range in cm (200000 = 2 km). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float Range = 200000.f;

	/** Cone half-angle in degrees when firing from the hip. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float HipSpreadDegrees = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Fire")
	float ScopedSpreadDegrees = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Scope")
	float HipFOV = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Scope")
	float ScopedFOV = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Scope")
	float ZoomInterpSpeed = 14.f;

	/** Drift speed of the crosshair while scoped, in degrees per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Handling")
	float SwayDegreesPerSecond = 1.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Handling")
	float BreathHoldSwayMultiplier = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Handling")
	float CrouchSwayMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Handling")
	float BreathHoldDuration = 4.f;

	/** How fast the breath meter refills, relative to real time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Handling")
	float BreathRecoveryRate = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilPitchDegrees = 2.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilYawDegrees = 0.5f;

	/** Fraction of the pitch kick that is pulled back down after the shot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilRecoveryFraction = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Recoil")
	float RecoilRecoverySpeed = 3.f;

	// ---- Actions ---------------------------------------------------------

	void SetScoped(bool bScoped) { bIsScoped = bScoped; }
	void SetBreathKeyHeld(bool bHeld) { bBreathKeyHeld = bHeld; }

	/** Returns true if a shot was fired. */
	bool TryFire();
	void StartReload();

	// ---- State for HUD / character --------------------------------------

	bool IsScoped() const { return bIsScoped; }
	bool IsReloading() const { return bIsReloading; }
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	/** 0 = fully hip, 1 = fully scoped. */
	float GetScopeAlpha() const;

	/** Look-input multiplier so a zoomed view turns more slowly. */
	float GetLookSensitivityScale() const;

	float GetBreathFraction() const;
	float GetLastHitTime() const { return LastHitTime; }
	bool WasLastHitHeadshot() const { return bLastHitHeadshot; }
	bool WasLastHitKill() const { return bLastHitKill; }

	FOnWeaponFired OnWeaponFired;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void FinishReload();
	void ApplyViewDelta(float YawDelta, float PitchDelta);

	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> Camera;

	int32 CurrentAmmo = 0;
	int32 ReserveAmmo = 0;
	float CurrentFOV = 90.f;
	float NextShotTime = 0.f;
	float PendingRecoilRecovery = 0.f;
	float BreathRemaining = 0.f;
	float LastHitTime = -100.f;

	bool bIsScoped = false;
	bool bIsReloading = false;
	bool bBreathKeyHeld = false;
	bool bHoldingBreath = false;
	bool bBreathExhausted = false;
	bool bLastHitHeadshot = false;
	bool bLastHitKill = false;

	FTimerHandle ReloadTimerHandle;
};
