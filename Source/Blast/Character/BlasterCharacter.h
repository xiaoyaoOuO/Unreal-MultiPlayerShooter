// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/TurningInPlace/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"


UCLASS()
class BLAST_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage();

private:
	UPROPERTY(VisibleAnywhere , Category= Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere , Category= Camera)
	class UCameraComponent * FollowCamera;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess=true))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;

	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed();
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	FRotator StartingAimRotation;
	
	UPROPERTY(Replicated)
	float AO_Yaw;

	float Interp_AO_Yaw;
	
	float AO_Pitch;

	UPROPERTY(Replicated)
	ETurningInPlace TurningInPlace;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* FireWeaponMontage;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AImButtonReleased();
	void TurnInPlace(float DeltaTime);
	void AimOffset(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	float Get_AO_Yaw() const {return AO_Yaw;}
	float Get_AO_Pitch() const {return AO_Pitch;}
	ETurningInPlace Get_TurningInPlace() const {return TurningInPlace;}
	AWeapon* Get_EquippedWeapon();
};
