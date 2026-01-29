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

		if (this->HUD.CheckNull())
		{
			DrawCrosshairs(HUD.CrosshairsCenter, ViewPortCenter);
			DrawCrosshairs(HUD.CrosshairsLeft, ViewPortCenter);
			DrawCrosshairs(HUD.CrosshairsRight, ViewPortCenter);
			DrawCrosshairs(HUD.CrosshairsTop, ViewPortCenter);
			DrawCrosshairs(HUD.CrosshairsBottom, ViewPortCenter);
		}
	}
}

void ABlasterHUD::DrawCrosshairs(UTexture2D* ToDraw, const FVector2D& ViewPortCenter)
{
	if (ToDraw)
	{
		int32 TextureX = ToDraw->GetSizeX();
		int32 TextureY = ToDraw->GetSizeY();
		DrawTexture(
			ToDraw,
			ViewPortCenter.X - (TextureX / 2.f),
			ViewPortCenter.Y - (TextureY / 2.f),
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
