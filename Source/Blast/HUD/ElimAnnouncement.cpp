// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"

void UElimAnnouncement::SetAnnouncementText(const FString& AttackerName, const FString& VictimName)
{
	FString ElimAnnouncementText = FString::Printf(TEXT("%s 击杀了 %s"), *AttackerName, *VictimName);
	if (AnnouncementText)
	{
		AnnouncementText->SetText(FText::FromString(ElimAnnouncementText));
	}
}
