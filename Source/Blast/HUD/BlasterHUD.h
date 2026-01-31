// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
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
	float CrosshairSpreadMax = 16.f;
public:
	virtual void DrawHUD() override;
	void DrawCrosshairs(UTexture2D* ToDraw, const FVector2D& ViewPortCenter,FLinearColor DrawColor,FVector2D Spread = FVector2D::ZeroVector);
	
	FORCEINLINE void SetHUDPackage(const FHUDPackage& HUDPackage){HUD = HUDPackage;};
};
