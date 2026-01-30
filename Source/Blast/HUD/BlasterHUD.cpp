// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2D ViewPortSize = FVector2D::ZeroVector;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
		const FVector2D ViewPortCenter(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);

		float SpreadScaled = HUD.SpreadSize * CrosshairSpreadMax;
		FVector2D Spread = FVector2D::ZeroVector;

		if (this->HUD.CheckNull())
		{
			DrawCrosshairs(HUD.CrosshairsCenter, ViewPortCenter);
			Spread.X = -SpreadScaled;
			DrawCrosshairs(HUD.CrosshairsLeft, ViewPortCenter,Spread);
			Spread.X = SpreadScaled;
			DrawCrosshairs(HUD.CrosshairsRight, ViewPortCenter,Spread);
			Spread.X = 0.f;
			Spread.Y = -SpreadScaled;
			DrawCrosshairs(HUD.CrosshairsTop, ViewPortCenter,Spread);
			Spread.Y = SpreadScaled;
			DrawCrosshairs(HUD.CrosshairsBottom, ViewPortCenter,Spread);
		}
	}
}

void ABlasterHUD::DrawCrosshairs(UTexture2D* ToDraw, const FVector2D& ViewPortCenter, FVector2D Spread)
{
	if (ToDraw)
	{
		int32 TextureX = ToDraw->GetSizeX();
		int32 TextureY = ToDraw->GetSizeY();
		DrawTexture(
			ToDraw,
			ViewPortCenter.X - (TextureX / 2.f) + Spread.X,
			ViewPortCenter.Y - (TextureY / 2.f) + Spread.Y,
			TextureX,
			TextureY,
			0.f,
			0.f,
			1.f,
			1.f,
			FLinearColor::White
		);
	}
}
