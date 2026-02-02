// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/HUD/BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

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
public:
	void SetBlasterPlayerHealth(float Health,float MaxHealth);
};
