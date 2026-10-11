#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/TimerHandle.h"
#include "SniperCharacter.generated.h"

class UCameraComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class USniperWeaponComponent;

/** First-person player pawn carrying the sniper rifle. */
UCLASS()
class SNIPERSHOOTER_API ASniperCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASniperCharacter();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	bool IsDead() const { return bDead; }
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	float GetHealthFraction() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	USniperWeaponComponent* GetWeapon() const { return Weapon; }
	UCameraComponent* GetCamera() const { return Camera; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
	float SprintSpeed = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player|Movement")
	float ScopedWalkSpeed = 150.f;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

	void JumpPressed();
	void JumpReleased();
	void FirePressed();
	void ScopePressed();
	void ScopeReleased();
	void ReloadPressed();
	void SprintPressed();
	void SprintReleased();
	void CrouchPressed();
	void CrouchReleased();
	void RestartPressed();

	void HandleWeaponFired();
	void HideMuzzleFlash();
	void Die();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RifleMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ScopeMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPointLightComponent> MuzzleFlash;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USniperWeaponComponent> Weapon;

	float Health = 100.f;
	bool bDead = false;
	bool bSprintHeld = false;

	FTimerHandle MuzzleFlashTimer;
};
