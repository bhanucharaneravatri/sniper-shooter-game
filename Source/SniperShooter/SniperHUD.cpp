#include "SniperHUD.h"
#include "SniperCharacter.h"
#include "SniperGameMode.h"
#include "SniperWeaponComponent.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"

void ASniperHUD::DrawHUD()
{
	Super::DrawHUD();

	const ASniperCharacter* Player = Cast<ASniperCharacter>(GetOwningPawn());
	if (!Canvas || !Player || !Player->GetWeapon())
	{
		return;
	}

	const USniperWeaponComponent* Weapon = Player->GetWeapon();
	const ASniperGameMode* GameMode = GetWorld()->GetAuthGameMode<ASniperGameMode>();

	const float Width = Canvas->SizeX;
	const float Height = Canvas->SizeY;
	const float CenterX = Width * 0.5f;
	const float CenterY = Height * 0.5f;

	const bool bScopeOverlay = Weapon->GetScopeAlpha() > 0.9f && !Player->IsDead();
	if (bScopeOverlay)
	{
		DrawScopeOverlay(Width, Height);
	}
	else if (!Player->IsDead())
	{
		DrawCrosshair(CenterX, CenterY);
	}

	// Hit marker: white for a hit, red for a headshot or kill.
	const float SinceHit = GetWorld()->GetTimeSeconds() - Weapon->GetLastHitTime();
	if (SinceHit < 0.3f)
	{
		const bool bSpecial = Weapon->WasLastHitHeadshot() || Weapon->WasLastHitKill();
		DrawHitMarker(CenterX, CenterY, bSpecial ? FLinearColor::Red : FLinearColor::White);
	}

	// Health, bottom left.
	DrawText(TEXT("HEALTH"), FLinearColor::White, 24.f, Height - 70.f, nullptr, 1.f);
	const float HealthFraction = Player->GetHealthFraction();
	DrawBar(24.f, Height - 48.f, 240.f, 16.f, HealthFraction, FLinearColor(1.f - HealthFraction, HealthFraction, 0.1f));

	// Ammo, bottom right.
	const FString AmmoText = FString::Printf(TEXT("%d / %d"), Weapon->GetCurrentAmmo(), Weapon->GetReserveAmmo());
	DrawText(AmmoText, FLinearColor::White, Width - 190.f, Height - 70.f, nullptr, 2.f);
	if (Weapon->IsReloading())
	{
		DrawText(TEXT("RELOADING..."), FLinearColor::Yellow, Width - 190.f, Height - 95.f, nullptr, 1.2f);
	}
	else if (Weapon->GetCurrentAmmo() == 0 && Weapon->GetReserveAmmo() == 0)
	{
		DrawText(TEXT("NO AMMO"), FLinearColor::Red, Width - 190.f, Height - 95.f, nullptr, 1.2f);
	}
	else if (Weapon->GetCurrentAmmo() == 0)
	{
		DrawText(TEXT("PRESS R TO RELOAD"), FLinearColor::Yellow, Width - 260.f, Height - 95.f, nullptr, 1.2f);
	}

	// Breath meter while scoped.
	if (Weapon->IsScoped())
	{
		DrawBar(CenterX - 80.f, Height - 40.f, 160.f, 8.f, Weapon->GetBreathFraction(), FLinearColor(0.5f, 0.8f, 1.f));
		DrawCenteredText(TEXT("HOLD SHIFT TO STEADY"), CenterX, Height - 62.f, FLinearColor(0.5f, 0.8f, 1.f), 1.f);
	}

	// Mission status, top centre.
	if (GameMode)
	{
		const FString MissionText = FString::Printf(TEXT("TARGETS %d / %d"), GameMode->GetKills(), GameMode->GetTotalEnemies());
		DrawCenteredText(MissionText, CenterX, 20.f, FLinearColor::White, 1.5f);

		if (GameMode->IsMissionComplete())
		{
			DrawCenteredText(TEXT("MISSION COMPLETE"), CenterX, CenterY - 60.f, FLinearColor::Green, 3.f);
			DrawCenteredText(TEXT("Press ENTER to restart"), CenterX, CenterY, FLinearColor::White, 1.5f);
		}
		else if (GameMode->IsPlayerDead())
		{
			DrawRect(FLinearColor(0.4f, 0.f, 0.f, 0.35f), 0.f, 0.f, Width, Height);
			DrawCenteredText(TEXT("YOU DIED"), CenterX, CenterY - 60.f, FLinearColor::Red, 3.f);
			DrawCenteredText(TEXT("Press ENTER to restart"), CenterX, CenterY, FLinearColor::White, 1.5f);
		}
	}

	// Controls hint, top left.
	DrawText(TEXT("WASD move | RMB scope | LMB fire | R reload | SHIFT sprint / steady | CTRL crouch"),
		FLinearColor(1.f, 1.f, 1.f, 0.6f), 20.f, 20.f, nullptr, 0.8f);
}

void ASniperHUD::DrawScopeOverlay(float Width, float Height)
{
	const float CenterX = Width * 0.5f;
	const float CenterY = Height * 0.5f;
	const float Radius = Height * 0.46f;

	// Black mask everywhere outside the circular lens.
	const float Step = 4.f;
	for (float Y = CenterY - Radius; Y < CenterY + Radius; Y += Step)
	{
		const float Dy = FMath::Abs((Y + Step * 0.5f) - CenterY);
		const float Half = FMath::Sqrt(FMath::Max(0.f, Radius * Radius - Dy * Dy));
		DrawRect(FLinearColor::Black, 0.f, Y, CenterX - Half, Step);
		DrawRect(FLinearColor::Black, CenterX + Half, Y, Width - (CenterX + Half), Step);
	}
	DrawRect(FLinearColor::Black, 0.f, 0.f, Width, CenterY - Radius);
	DrawRect(FLinearColor::Black, 0.f, CenterY + Radius, Width, Height - (CenterY + Radius));

	// Lens rim.
	const int32 Segments = 72;
	for (int32 i = 0; i < Segments; ++i)
	{
		const float A0 = 2.f * PI * i / Segments;
		const float A1 = 2.f * PI * (i + 1) / Segments;
		DrawLine(CenterX + FMath::Cos(A0) * Radius, CenterY + FMath::Sin(A0) * Radius,
			CenterX + FMath::Cos(A1) * Radius, CenterY + FMath::Sin(A1) * Radius, FLinearColor::Black, 3.f);
	}

	// Duplex reticle: thick outer posts, fine centre cross, mil-dot ticks.
	const float Gap = 24.f;
	DrawLine(CenterX - Radius, CenterY, CenterX - Gap, CenterY, FLinearColor::Black, 3.f);
	DrawLine(CenterX + Gap, CenterY, CenterX + Radius, CenterY, FLinearColor::Black, 3.f);
	DrawLine(CenterX, CenterY - Radius, CenterX, CenterY - Gap, FLinearColor::Black, 3.f);
	DrawLine(CenterX, CenterY + Gap, CenterX, CenterY + Radius, FLinearColor::Black, 3.f);
	DrawLine(CenterX - Gap, CenterY, CenterX + Gap, CenterY, FLinearColor::Black, 1.f);
	DrawLine(CenterX, CenterY - Gap, CenterX, CenterY + Gap, FLinearColor::Black, 1.f);

	for (int32 i = 1; i <= 5; ++i)
	{
		const float Offset = 45.f * i;
		DrawLine(CenterX - Offset, CenterY - 5.f, CenterX - Offset, CenterY + 5.f, FLinearColor::Black, 1.f);
		DrawLine(CenterX + Offset, CenterY - 5.f, CenterX + Offset, CenterY + 5.f, FLinearColor::Black, 1.f);
		DrawLine(CenterX - 5.f, CenterY + Offset, CenterX + 5.f, CenterY + Offset, FLinearColor::Black, 1.f);
	}
}

void ASniperHUD::DrawCrosshair(float CenterX, float CenterY)
{
	const float Gap = 6.f;
	const float Length = 10.f;
	const FLinearColor Color = FLinearColor::White;
	DrawLine(CenterX - Gap - Length, CenterY, CenterX - Gap, CenterY, Color, 2.f);
	DrawLine(CenterX + Gap, CenterY, CenterX + Gap + Length, CenterY, Color, 2.f);
	DrawLine(CenterX, CenterY - Gap - Length, CenterX, CenterY - Gap, Color, 2.f);
	DrawLine(CenterX, CenterY + Gap, CenterX, CenterY + Gap + Length, Color, 2.f);
}

void ASniperHUD::DrawHitMarker(float CenterX, float CenterY, const FLinearColor& Color)
{
	const float Near = 8.f;
	const float Far = 18.f;
	DrawLine(CenterX - Far, CenterY - Far, CenterX - Near, CenterY - Near, Color, 2.f);
	DrawLine(CenterX + Far, CenterY - Far, CenterX + Near, CenterY - Near, Color, 2.f);
	DrawLine(CenterX - Far, CenterY + Far, CenterX - Near, CenterY + Near, Color, 2.f);
	DrawLine(CenterX + Far, CenterY + Far, CenterX + Near, CenterY + Near, Color, 2.f);
}

void ASniperHUD::DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& FillColor)
{
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), X - 2.f, Y - 2.f, Width + 4.f, Height + 4.f);
	DrawRect(FillColor, X, Y, Width * FMath::Clamp(Fraction, 0.f, 1.f), Height);
}

void ASniperHUD::DrawCenteredText(const FString& Text, float CenterX, float Y, const FLinearColor& Color, float Scale)
{
	float TextWidth = 0.f;
	float TextHeight = 0.f;
	GetTextSize(Text, TextWidth, TextHeight, nullptr, Scale);
	DrawText(Text, Color, CenterX - TextWidth * 0.5f, Y, nullptr, Scale);
}
