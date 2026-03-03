// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MultiPlayerSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

public:
	void MenuStart();
	
	void MenuTearDown();

	UFUNCTION()
	void OnReturnButtonClicked();
	
	UFUNCTION()
	void OnDestroySessionComplete(bool bWasSuccessful);

	
	UPROPERTY(meta=(BindWidget))
	UButton* ReturnButton;

	UPROPERTY()
	UMultiPlayerSessionSubsystem* MultiplayerSessionsSubsystem;
	
};
