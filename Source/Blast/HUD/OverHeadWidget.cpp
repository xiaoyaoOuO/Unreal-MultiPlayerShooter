// Fill out your copyright notice in the Description page of Project Settings.


#include "OverHeadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverHeadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();
}

void UOverHeadWidget::SetDisplayText(FString text)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(text));
	}
}

void UOverHeadWidget::ShowPlayerName(APawn* Inpawn)
{
	APlayerState* PlayerState = Inpawn->GetPlayerState();
	FString PlayerName;
	if (PlayerState)
	{
		PlayerName = PlayerState->GetPlayerName();
		SetDisplayText(PlayerName);
	}
}
