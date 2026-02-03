// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"

#include "Blast/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/SkeletalMeshSocket.h"

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
		if (BlasterCharacter == nullptr) return;
	}

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.0f;
	Speed = Velocity.Size();//只看2维变量，用于idle/walk/run转换

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	bIsEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->Get_EquippedWeapon(); //获取手持武器，将手与武器绑定好
	bIsCrouch = BlasterCharacter->bIsCrouched;
	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->Get_TurningInPlace();
	bElimmed = BlasterCharacter->ShouldElim();

	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MoveRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	FRotator DeltaRo = UKismetMathLibrary::NormalizedDeltaRotator(MoveRotation,AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,DeltaRo,DeltaTime,15.f);
	YawOffset = DeltaRotation.Yaw;


	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean,Target,DeltaTime,6.f);
	Lean = FMath::Clamp(Interp,-90.f,90.f);

	AO_Yaw = BlasterCharacter->Get_AO_Yaw();
	AO_Pitch = BlasterCharacter->Get_AO_Pitch();
	bRotateRootBone = BlasterCharacter->Get_bRotateRootBone();

	USkeletalMeshComponent* EquippedWeaponMesh;
    USkeletalMeshComponent* BlasterCharacterMesh = BlasterCharacter->GetMesh();
	if (bIsEquipped && EquippedWeapon && (EquippedWeaponMesh = EquippedWeapon->Get_WeaponMesh()) && BlasterCharacterMesh)
	{
		LeftHandTransform = EquippedWeaponMesh->GetSocketTransform(FName("LeftHandSocket"));
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacterMesh->TransformToBoneSpace(FName("hand_r"),LeftHandTransform.GetLocation(),FRotator::ZeroRotator,OutPosition,OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
		
		/*
		 * 修正武器朝向,TODO：复制玩家目前不会修正
		*/
		if (const USkeletalMeshSocket* MuzzleFlash = EquippedWeaponMesh->GetSocketByName("MuzzleFlash"))
		{
			if (BlasterCharacter->IsLocallyControlled())
			{
				bIsLocalControl = true;
				// const FTransform WeaponSocketTransform = MuzzleFlash->GetSocketTransform(EquippedWeaponMesh);
				const FTransform RightHandTransform = BlasterCharacterMesh->GetSocketTransform("Hand_R");
				//获取瞄准方向
				FVector HitTarget = BlasterCharacter->Get_HitResult();
				//右手的X轴朝向身体方向，所以rotation取反
				TargetRotation = UKismetMathLibrary::FindLookAtRotation(
					RightHandTransform.GetLocation(),
					RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() -HitTarget)
				);
				// DrawDebugLine(
				// 	GetWorld(),WeaponSocketTransform.GetLocation(),WeaponSocketTransform.GetLocation() + WeaponSocketTransform.GetRotation().Vector()*100.f,FColor::Red
				// );
				// DrawDebugLine(
				// 	GetWorld(),WeaponSocketTransform.GetLocation(),HitTarget,FColor::Blue
				// );
			}
		}
	}
}

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}
