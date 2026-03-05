// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ElimAnnouncement.h"
#include "GameFramework/HUD.h"
#include "Blast/HUD/CharacterOverlay.h"
#include "Blast/HUD/Announcement.h"
#include "BlasterHUD.generated.h"


USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UTexture2D* CrosshairsCenter;
	UTexture2D* CrosshairsLeft;
	UTexture2D* CrosshairsRight;
	UTexture2D* CrosshairsTop;
	UTexture2D* CrosshairsBottom;
	FLinearColor CrosshairColor;

	float SpreadSize;

	bool CheckNull() const
	{return  CrosshairsCenter != nullptr && CrosshairsLeft != nullptr &&
		CrosshairsRight != nullptr && CrosshairsTop != nullptr && CrosshairsBottom != nullptr;}
};

UCLASS()
class BLAST_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

private:
	FHUDPackage HUD;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ElimAnnouncementClass;

	TArray<UElimAnnouncement*> AnnouncementArray;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;
	float CrosshairSpreadMax = 16.f;

	UPROPERTY()
	UAnnouncement* Announcement;

	UPROPERTY(EditAnywhere)
	float ElimAnnouncementDuration = 2.5f;
	
public:
	virtual void BeginPlay() override;
	virtual void DrawHUD() override;
	void DrawCrosshairs(UTexture2D* ToDraw, const FVector2D& ViewPortCenter,FLinearColor DrawColor,FVector2D Spread = FVector2D::ZeroVector);
	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(const FString& AttackerName, const FString& VictimPlayerName);
	void CloseAnnouncement();
	void OpenAnnouncement();

	void HideTeamScore();
	void InitTeamScore();
	void UpdateTeamScore(int RedTeamScore, int BlueTeamScore);

	UFUNCTION()
	void RemoveElimAnnouncement(UElimAnnouncement* AnnouncementToRemove);
	
	FORCEINLINE void SetHUDPackage(const FHUDPackage& HUDPackage){HUD = HUDPackage;}
	FORCEINLINE UCharacterOverlay* GetCharacterOverlay() const {return CharacterOverlay;}
	FORCEINLINE UAnnouncement* GetAnnouncement() const {return Announcement;}
};