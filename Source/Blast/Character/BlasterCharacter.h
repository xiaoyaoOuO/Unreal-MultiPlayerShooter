// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/TurningInPlace/TurningInPlace.h"
#include "GameFramework/Character.h"
#include "Blast/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blast/TurningInPlace/CombatState.h"
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

	//蒙太奇动画
	void PlayFireMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlayReloadMontage();
	void PlayGrenadeMontage();

private:
	UPROPERTY(VisibleAnywhere , Category= Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere , Category= Camera)
	class UCameraComponent * FollowCamera;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess=true))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing=OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess=true))
	class UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess=true))
	class UBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere)
	class ABlasterPlayerController* BlasterPlayerController;

	UPROPERTY(VisibleAnywhere)
	class ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* GrenadeMesh;

	UFUNCTION(Server,Reliable)
	void ServerEquipButtonPressed();
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

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

	UPROPERTY()
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

	UPROPERTY(EditAnywhere,Category= "Character Stats")
	int8 MaxShield = 50;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentShield,VisibleAnywhere,Category= "Character Stats")
	int8 CurrentShield;
	
	UPROPERTY(Replicated)
	ETurningInPlace TurningInPlace;

	/*
	 * 人物的动画蒙太奇
	 */
	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere,Category=Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	UAnimMontage* GrenadeTossMontage;


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
	/*
	 * 操作绑定函数
	 */
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AImButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void GrenadeButtonPressed();
	
	void TurnInPlace(float DeltaTime);
	void Calculate_AO_Pitch();
	void AimOffset(float DeltaTime);
	void HideCharacterWhenCameraClose();
	void SimProxiesTurn();
	void RespawnTimerFinished();
	

	float Calculate_Speed();

	UFUNCTION()
	void OnRep_CurrentHealth(float LastHealth);

	UFUNCTION()
	void OnRep_CurrentShield();

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
	void UpdateHealthHUD();
	void UpdateShieldHUD();
	void PollInit();   //在Tick中做检查，如果没有初始化就初始化
	void JumpToShotgunEnd();  //按个数添加散弹枪子弹，当满子弹后就跳跃至End
	void SetGrenadeVisibility(bool bVisible);
	
	AWeapon* Get_EquippedWeapon();
	FVector  Get_HitResult();
	ECombatState Get_CombatState() const;
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScopeWidget);
	
	
	FORCEINLINE float Get_AO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float Get_AO_Pitch() const {return AO_Pitch;}
	FORCEINLINE bool  Get_bRotateRootBone() const {return bRotateRootBone;}
	FORCEINLINE bool  ShouldElim() const {return bShouldElim;}
	FORCEINLINE float Get_CurrentHealth() const {return CurrentHealth;}
	FORCEINLINE float Get_MaxHealth() const {return MaxHealth;}
	FORCEINLINE void Set_CurrentHealth(float Health) {CurrentHealth = Health;}
	FORCEINLINE ETurningInPlace Get_TurningInPlace() const {return TurningInPlace;}
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera;}
	FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() const {return GrenadeMesh;}
	FORCEINLINE UCombatComponent* GetCombatComponent() const {return CombatComponent;}
	FORCEINLINE UBuffComponent* GetBuffComponent() const {return BuffComponent;}
};
