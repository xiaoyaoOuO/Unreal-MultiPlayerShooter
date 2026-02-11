// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Announcement.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WarmUpTimerText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TitleText;           //cooldown状态和warmup状态下的标题
	
};
