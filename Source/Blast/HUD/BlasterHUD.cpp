// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "Blast/PlayerController/BlasterPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
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

void ABlasterHUD::AddElimAnnouncement(const FString& AttackerName, const FString& VictimPlayerName)
{
	ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetOwningPlayerController());
	if (ElimAnnouncementClass)
	{
		if (UElimAnnouncement* ElimAnnouncement = CreateWidget<UElimAnnouncement>(PlayerController,ElimAnnouncementClass))
		{
			ElimAnnouncement->SetAnnouncementText(AttackerName,VictimPlayerName);
			ElimAnnouncement->AddToViewport();

			//如果有多条公告，按序排列
			for (auto ElimAnnouncementInArray : AnnouncementArray)
			{
				if (ElimAnnouncementInArray == nullptr) continue;
				if (UCanvasPanelSlot* CanvasSlot =UWidgetLayoutLibrary::SlotAsCanvasSlot(ElimAnnouncementInArray->AnnouncementBox))
				{
					FVector2D CurrentPosition = CanvasSlot->GetPosition();
					FVector2D NewPosition = FVector2D(
						CurrentPosition.X,
						CurrentPosition.Y - CanvasSlot->GetSize().Y 
					);
					CanvasSlot->SetPosition(NewPosition);
				}
			}
			AnnouncementArray.Add(ElimAnnouncement);

			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUFunction(this,FName("RemoveElimAnnouncement"),ElimAnnouncement);
			GetWorld()->GetTimerManager().SetTimer(TimerHandle,TimerDelegate,ElimAnnouncementDuration,false);
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

void ABlasterHUD::HideTeamScore()
{
	if (CharacterOverlay)
	{
		if (CharacterOverlay->BlueTeamScoreText)
		{
			CharacterOverlay->BlueTeamScoreText->SetVisibility(ESlateVisibility::Hidden);
		}
		if (CharacterOverlay->RedTeamScoreText)
		{
			CharacterOverlay->RedTeamScoreText->SetVisibility(ESlateVisibility::Hidden);
		}
		if (CharacterOverlay->TeamScoreSlashText)
		{
			CharacterOverlay->TeamScoreSlashText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterHUD::InitTeamScore()
{
	if (CharacterOverlay)
	{
		if (CharacterOverlay->BlueTeamScoreText)
		{
			CharacterOverlay->BlueTeamScoreText->SetVisibility(ESlateVisibility::Visible);
			CharacterOverlay->BlueTeamScoreText->SetText(FText::FromString("0"));
		}
		if (CharacterOverlay->RedTeamScoreText)
		{
			CharacterOverlay->RedTeamScoreText->SetVisibility(ESlateVisibility::Visible);
			CharacterOverlay->RedTeamScoreText->SetText(FText::FromString("0"));
		}
		if (CharacterOverlay->TeamScoreSlashText)
		{
			CharacterOverlay->TeamScoreSlashText->SetVisibility(ESlateVisibility::Visible);
			CharacterOverlay->TeamScoreSlashText->SetText(FText::FromString("|"));
		}
	}
}

void ABlasterHUD::UpdateTeamScore(int RedTeamScore, int BlueTeamScore)
{
	if (CharacterOverlay)
	{
		if (CharacterOverlay->BlueTeamScoreText)
		{
			FString BlueTeamScoreString = FString::Printf(TEXT("%d"), BlueTeamScore);
			CharacterOverlay->BlueTeamScoreText->SetText(FText::FromString(BlueTeamScoreString));
		}
		if (CharacterOverlay->RedTeamScoreText)
		{
			FString RedTeamScoreString = FString::FromInt(RedTeamScore);
			CharacterOverlay->RedTeamScoreText->SetText(FText::FromString(RedTeamScoreString));
		}
	}
}

void ABlasterHUD::RemoveElimAnnouncement(UElimAnnouncement* AnnouncementToRemove)
{
	if (AnnouncementToRemove)
	{
		if (AnnouncementArray.Contains(AnnouncementToRemove))
		{
			AnnouncementArray.Remove(AnnouncementToRemove);
		}
		AnnouncementToRemove->RemoveFromParent();
	}
}
