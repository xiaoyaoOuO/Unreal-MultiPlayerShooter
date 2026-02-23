// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* HealthText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ScoreText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* DefeatText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AmmoAmountText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CarriedAmmoAmountText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CountDownText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* GrenadeAmountText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* PingText;

	UPROPERTY(meta=(BindWidget))
	UImage* HighPingImage;
	
	UPROPERTY(meta=(BindWidgetAnim),Transient)
	UWidgetAnimation* HighPingWarningAnimation;
};
