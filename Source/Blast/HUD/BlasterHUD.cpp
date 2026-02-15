// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

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

		FLinearColor DrawColor = HUD.CrosshairColor;
		if (HUD.CrosshairsCenter)
		{
			DrawCrosshairs(HUD.CrosshairsCenter, ViewPortCenter,DrawColor,Spread);
		}
		Spread.X = -SpreadScaled;
		if (HUD.CrosshairsLeft)
		{
			DrawCrosshairs(HUD.CrosshairsLeft, ViewPortCenter,DrawColor,Spread);
		}
		Spread.X = SpreadScaled;
		if (HUD.CrosshairsRight)
		{
			DrawCrosshairs(HUD.CrosshairsRight, ViewPortCenter,DrawColor,Spread);
		}
		Spread.X = 0.f;
		Spread.Y = -SpreadScaled;
		if (HUD.CrosshairsTop)
		{
			DrawCrosshairs(HUD.CrosshairsTop, ViewPortCenter,DrawColor,Spread);
		}
		Spread.Y = SpreadScaled;
		if (HUD.CrosshairsBottom)
		{
			DrawCrosshairs(HUD.CrosshairsBottom, ViewPortCenter,DrawColor,Spread);
		}
	}
}

void ABlasterHUD::DrawCrosshairs(UTexture2D* ToDraw, const FVector2D& ViewPortCenter, FLinearColor DrawColor ,FVector2D Spread)
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
			DrawColor
		);
	}
}

void ABlasterHUD::AddCharacterOverlay()
{
	if (CharacterOverlay) return;
	if (APlayerController* PlayerController = GetOwningPlayerController())
	{
		if (CharacterOverlayClass)
		{
			CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController,CharacterOverlayClass);
			if (CharacterOverlay)
			{
				UE_LOG(LogTemp,Warning,TEXT("CharacterOverlay Added to Viewport"));
				CharacterOverlay->AddToViewport();
			}
		}
	}
}

void ABlasterHUD::AddAnnouncement()
{
	if (Announcement) return;
	if (APlayerController* PlayerController = GetOwningPlayerController())
	{
		if (AnnouncementClass)
		{
			Announcement = CreateWidget<UAnnouncement>(PlayerController,AnnouncementClass);
			if (Announcement)
			{
				UE_LOG(LogTemp,Warning,TEXT("Announcement Added to Viewport"));
				Announcement->AddToViewport();
			}
		}
	}
}

void ABlasterHUD::CloseAnnouncement()
{
	if (Announcement)
	{
		Announcement->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ABlasterHUD::OpenAnnouncement()
{
	if (Announcement)
	{
		Announcement->SetVisibility(ESlateVisibility::Visible);
	}
}
