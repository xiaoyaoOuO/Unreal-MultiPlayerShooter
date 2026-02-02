// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/TurningInPlace/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "Blast/Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterCharacter.generated.h"


UCLASS()
class BLAST_API ABlasterCharacter : public ACharacter , public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage();
	void PlayHitReactMontage();

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

	UPROPERTY(VisibleAnywhere)
	class ABlasterPlayerController* BlasterPlayerController;

	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed();
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	FRotator StartingAimRotation;
	
	UPROPERTY(Replicated)
	float AO_Yaw;

	float Interp_AO_Yaw;
	
	float AO_Pitch;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	/*
	 * 优化网络传输，人物转向只在服务端和非拥有客户端进行
	 */
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	FRotator ProxyLastRotationFrame;
	FRotator ProxyRotationFrame;

	/*
	 * 玩家状态
	 */
	UPROPERTY(EditAnywhere,Category= "Character Stats")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentHealth,VisibleAnywhere,Category= "Character Stats")
	float CurrentHealth;
	
	UPROPERTY(Replicated)
	ETurningInPlace TurningInPlace;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* HitReactMontage;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnRep_ReplicatedMovement() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AImButtonReleased();
	void TurnInPlace(float DeltaTime);
	void Calculate_AO_Pitch();
	void AimOffset(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();
	void HideCharacterWhenCameraClose();
	void SimProxiesTurn();

	float Calculate_Speed();

	UFUNCTION()
	void OnRep_CurrentHealth();

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon* Get_EquippedWeapon();
	FVector Get_HitResult();
	
	UFUNCTION(NetMulticast,Unreliable)
	void MulticastHitReact();
	
	FORCEINLINE float Get_AO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float Get_AO_Pitch() const {return AO_Pitch;}
	FORCEINLINE bool  Get_bRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE ETurningInPlace Get_TurningInPlace() const {return TurningInPlace;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera;}
};
