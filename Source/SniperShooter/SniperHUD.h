#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SniperHUD.generated.h"

/** Canvas HUD: crosshair, scope overlay, health, ammo, mission status, hit marker. */
UCLASS()
class SNIPERSHOOTER_API ASniperHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawScopeOverlay(float Width, float Height);
	void DrawCrosshair(float CenterX, float CenterY);
	void DrawHitMarker(float CenterX, float CenterY, const FLinearColor& Color);
	void DrawBar(float X, float Y, float Width, float Height, float Fraction, const FLinearColor& FillColor);
	void DrawCenteredText(const FString& Text, float CenterX, float Y, const FLinearColor& Color, float Scale);
};
