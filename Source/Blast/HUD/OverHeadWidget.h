// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLAST_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	void SetDisplayText(FString text);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerName(APawn* Inpawn);
	
};
