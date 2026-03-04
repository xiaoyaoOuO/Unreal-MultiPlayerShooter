// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "ElimAnnouncement.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* AnnouncementBox;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* AnnouncementText;

	void SetAnnouncementText(const FString& AttackerName, const FString& VictimName);
};
