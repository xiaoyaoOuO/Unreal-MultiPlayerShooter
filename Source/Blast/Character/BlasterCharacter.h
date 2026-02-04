// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/TurningInPlace/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "Blast/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
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
	void UpdateHealthHUD();

	FRotator StartingAimRotation;
	
	UPROPERTY(Replicated)
	float AO_Yaw;

	float Interp_AO_Yaw;
	
	float AO_Pitch;

	/*
	 * 死亡和重生
	 */
	bool bShouldElim;
	FTimerHandle RespawnTimer;
	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.f;

	/*
	 * 回收机器人
	 */
	UPROPERTY(EditAnywhere,Category= "Elim")
	UParticleSystem* ElimBotParticle;
	
	UPROPERTY(EditAnywhere,Category= "Elim")
	USoundCue* BotSoundCue;
	
	UParticleSystemComponent* ElimBot;

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

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* ElimMontage;


	/*
	 * 角色死亡时消融特效
	 */
	UPROPERTY(EditAnywhere,Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(VisibleDefaultsOnly,Category = Elim)
	UMaterialInstanceDynamic* DissolveMaterialInstanceDynamic;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTimelineTrack;

	UPROPERTY(EditAnywhere,Category = Elim)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;

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
	void PlayElimMontage();
	void RespawnTimerFinished();

	float Calculate_Speed();

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
    /*即 void(ABlasterCharacter::*)(AActor*,
     *float,
     *const UDamageType*,
     *AController*,
     *AActor*))，
	*/
	void ReceiveDamage(AActor* DamagedActor,float Damage,const UDamageType* DamageType,AController* InstigatedBy,AActor* DamageCauser);

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	void Elim();
	AWeapon* Get_EquippedWeapon();
	FVector  Get_HitResult();
	
	
	FORCEINLINE float Get_AO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float Get_AO_Pitch() const {return AO_Pitch;}
	FORCEINLINE bool  Get_bRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE bool  ShouldElim() const {return bShouldElim;}
	FORCEINLINE ETurningInPlace Get_TurningInPlace() const {return TurningInPlace;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera;}
};
