// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/Character/BlasterCharacter.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLAST_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	friend class ABlasterCharacter;
	void EquipWeapon(AWeapon* Weapon);
	void SetAiming(bool bIsAiming);
	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bIsAiming);

	UFUNCTION(NetMulticast,Reliable)
	void MulticastFire(FVector_NetQuantize HitTarget);

	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize HitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_EquippedWeapon();

public:
	ABlasterCharacter* Character;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseMoveSpeed;
	UPROPERTY(EditAnywhere)
	float AimingMoveSpeed;

	bool bFireButtonPressed;
};
