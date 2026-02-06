// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/HUD/BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

UENUM()
enum EHUDType
{
	EHT_CarriedAmmo,
	EHT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT()
struct FHUDData
{
	GENERATED_BODY()
	int32 CarriedAmmo;
};

/**
 * 
 */
UCLASS()
class BLAST_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	ABlasterHUD* BlasterHUD;
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
public:
	void SetBlasterPlayerHealth(float Health,float MaxHealth);
	void SetBlasterPlayerScore(float Score);
	void SetBlasterPlayerDefeat(int32 Defeat);
	void SetBlasterPlayerAmmoAmount(int32 AmmoAmount);
	void SetBlasterPlayerHUDData(const EHUDType& HUDType,const FHUDData& Data);
};
