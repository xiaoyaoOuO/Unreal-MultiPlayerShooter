// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Kismet/GameplayStatics.h"

ULagCompensationComponent::ULagCompensationComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

}



void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);
	ShowFramePackage(FramePackage);
}



void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character == nullptr || !Character->HasAuthority()) return;
	UpdateFrameHistory();
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	Package.HitCharacter = Character;
	if (Character)
	{
		for (const auto& HitBox : Character->HitBoxComponentMap)
		{
			FBoxInformation BoxInformation;
			Package.Time = GetWorld()->GetTimeSeconds();
			if (const UBoxComponent* Box = HitBox.Value)
			{
				BoxInformation.BoxLocation = Box->GetComponentLocation();
				BoxInformation.BoxRotation = Box->GetComponentRotation();
				BoxInformation.BoxExtent = Box->GetScaledBoxExtent();
			}
			Package.HitBoxInfo.Add(HitBox.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package)
{
	for (const auto& HitBox : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(),
			HitBox.Value.BoxLocation,
			HitBox.Value.BoxExtent,
			FQuat(HitBox.Value.BoxRotation),
			FColor::Red,
			false,
			MaxRecordTime
		);
	}
}

void ULagCompensationComponent::AddFrame()
{
	FFramePackage FramePackage;
	SaveFramePackage(FramePackage);
	FrameHistory.AddHead(FramePackage);
	// ShowFramePackage(FramePackage);
}

void ULagCompensationComponent::UpdateFrameHistory()
{
	if (FrameHistory.Num() <= 1)
	{
		AddFrame();
	}else
	{
		//先剔除掉超过记录最长时间的历史帧数据，再添加当前帧数据
		float HistoryRecordTime = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryRecordTime > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryRecordTime = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		AddFrame();
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerRewind(float HitTime, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, const ABlasterCharacter* HitCharacter)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitTime, HitCharacter);
	return ConfirmHit(FrameToCheck, TraceStart, HitLocation, HitCharacter);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerRewind(float HitTime,
                                                                              const FVector_NetQuantize& TraceStart,
                                                                              const TArray<FVector_NetQuantize>& HitLocations,
                                                                              const TArray<ABlasterCharacter*>& HitCharacters)
{
	TArray<FFramePackage> FramePackages;
	for (auto& HitCharacter : HitCharacters)
	{
		FramePackages.Add(GetFrameToCheck(HitTime, HitCharacter));
	}
	return ShotgunConfirmHit(FramePackages, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::FrameInterp(const FFramePackage& OldFrame, const FFramePackage& YoungFrame,float HitTime)
{
	FFramePackage InterpFrame;
	float TimeBetweenFrames = YoungFrame.Time - OldFrame.Time;
	float LerpFactor = (HitTime - OldFrame.Time) / TimeBetweenFrames;
	InterpFrame.Time = HitTime;
	for (const auto& HitBox : OldFrame.HitBoxInfo)
	{
		FBoxInformation InterpBoxInfo;
		FBoxInformation OldBoxInfo = HitBox.Value;
		FBoxInformation YoungBoxInfo = YoungFrame.HitBoxInfo[HitBox.Key];
		InterpBoxInfo.BoxExtent = FMath::VInterpTo(OldBoxInfo.BoxExtent, YoungBoxInfo.BoxExtent, 1.f, LerpFactor);
		InterpBoxInfo.BoxLocation = FMath::VInterpTo(OldBoxInfo.BoxLocation, YoungBoxInfo.BoxLocation, 1.f, LerpFactor);
		InterpBoxInfo.BoxRotation = FMath::RInterpTo(OldBoxInfo.BoxRotation, YoungBoxInfo.BoxRotation, 1.f, LerpFactor);
		InterpFrame.HitBoxInfo.Add(HitBox.Key, InterpBoxInfo);
	}
	return InterpFrame;
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(float HitTime, const ABlasterCharacter* HitCharacter)
{
	if (Character == nullptr) return FFramePackage();
	
	ULagCompensationComponent* LagCompensationComponent = HitCharacter->GetLagCompensationComponent();
	if (LagCompensationComponent == nullptr) return FFramePackage();

	auto HeadFrame = LagCompensationComponent->FrameHistory.GetHead();
	auto TailFrame = LagCompensationComponent->FrameHistory.GetTail();
	if (HeadFrame == nullptr || TailFrame == nullptr) return FFramePackage();

	FFramePackage FrameToCheck;
	FrameToCheck.HitCharacter = const_cast<ABlasterCharacter*>(HitCharacter);
	bool bShouldInterpolate = true;
	
	//超过记录的最长时间，无法回溯
	if (HitTime < TailFrame->GetValue().Time) return FFramePackage();

	if (HitTime == TailFrame->GetValue().Time)
	{
		FrameToCheck = TailFrame->GetValue();
		bShouldInterpolate = false;
	}
	if (HitTime >= HeadFrame->GetValue().Time)
	{
		FrameToCheck = HeadFrame->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Young = TailFrame;  
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Old = TailFrame;
	//从最旧的帧开始往前找，直到找到第一个新于HitTime的帧(新帧的时间大于HitTime)
	while (Young && HitTime >= Young->GetValue().Time)
	{
		Old = Young;
		Young = Young->GetPrevNode();
	}

	if (Old->GetValue().Time == HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = Old->GetValue();
	}

	if (bShouldInterpolate)
	{
		//插值计算出HitTime时的帧数据
		FrameToCheck = FrameInterp(Old->GetValue(), Young->GetValue(), HitTime);
	}
	return FrameToCheck;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
                                                              const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation,
                                                              const ABlasterCharacter* HitCharacter)
{
	FServerSideRewindResult Result{false, false};
	if (HitCharacter == nullptr) return Result;
	
	FFramePackage CurrentFrame;
	CacheCharacterHitBox(HitCharacter, CurrentFrame);
	MoveHitBoxes(Package, HitCharacter);
	SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision); //关闭角色Mesh的碰撞，防止射线检测时被Mesh挡住,后面一定要重置

	SetHeadHitBoxCollision(HitCharacter, ECollisionEnabled::QueryOnly);
	FHitResult HitResult;
	FVector HitEndLocation = TraceStart + (HitLocation - TraceStart) * 1.25f; //增加射线长度，防止射线过短无法击中
	if (UWorld* World = GetWorld())
	{
		World->LineTraceSingleByChannel(HitResult, TraceStart, HitEndLocation, ECC_Visibility);
		if (HitResult.bBlockingHit)
		{
			Result.bHeadShot = true;
		}else
		//如果没有击中头部，再检测身体其他部位
		{
			SetAllHitBoxCollision(HitCharacter, ECollisionEnabled::QueryOnly);
			World->LineTraceSingleByChannel(HitResult, TraceStart, HitEndLocation, ECC_Visibility);
			if (HitResult.bBlockingHit)
			{
				Result.bHeadShot = false;
			}
		}
	}
	//检测过后，重置所有被改变的属性
	SetAllHitBoxCollision(HitCharacter, ECollisionEnabled::NoCollision);
	SetCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	MoveHitBoxes(CurrentFrame, HitCharacter);
	Result.bConfirmed = true;
	return Result;
}

void ULagCompensationComponent::AddShotgunHeadShot(FShotgunServerSideRewindResult& ShotgunServerSideRewindResult, ABlasterCharacter* HitCharacter)
{
	if (ShotgunServerSideRewindResult.CharacterHeadShots.Contains(HitCharacter))
	{
		ShotgunServerSideRewindResult.CharacterHeadShots[HitCharacter]++;
	}else
	{
		ShotgunServerSideRewindResult.CharacterHeadShots.Add(HitCharacter,1);
	}
}

void ULagCompensationComponent::AddShotgunBodyShot(FShotgunServerSideRewindResult& ShotgunServerSideRewindResult, ABlasterCharacter* HitCharacter)
{
	if (ShotgunServerSideRewindResult.CharacterBodyShots.Contains(HitCharacter))
	{
		ShotgunServerSideRewindResult.CharacterBodyShots[HitCharacter]++;
	}else
	{
		ShotgunServerSideRewindResult.CharacterBodyShots.Add(HitCharacter,1);
	}
}

FShotgunServerSideRewindResult ULagCompensationComponent:: ShotgunConfirmHit(TArray<FFramePackage>& Packages,
                                                                             const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	FShotgunServerSideRewindResult ShotgunServerSideRewindResult{false};
	TArray<FFramePackage> CharacterOriginPackages;
	//对于每个可能被击中的角色，先把它们的Hitbox位置缓存起来，然后移动Hitbox到被击中时的位置，最后进行碰撞检测，检测过后再重置Hitbox位置
	for (auto& Package : Packages)
	{
		FFramePackage CurrentFrame;
		CacheCharacterHitBox(Package.HitCharacter,CurrentFrame);
		CharacterOriginPackages.Emplace(CurrentFrame);
		MoveHitBoxes(Package, Package.HitCharacter);
		SetCharacterMeshCollision(Package.HitCharacter, ECollisionEnabled::NoCollision); //关闭角色Mesh的碰撞，防止射线检测时被Mesh挡住,后面一定要重置
		SetAllHitBoxCollision(Package.HitCharacter, ECollisionEnabled::QueryOnly); //先把所有的Hitbox都设置成QueryOnly，如果射线检测到命中，再根据命中的Hitbox判断是爆头还是身体其他部位被击中
	}
	FHitResult HitResult;
	UWorld* World = GetWorld();
	for (auto& HitLocation : HitLocations)
	{
		FVector HitEndLocation = TraceStart + (HitLocation - TraceStart) * 1.25f; //增加射线长度，防止射线过短无法击中
		if (World)
		{
			World->LineTraceSingleByChannel(HitResult, TraceStart, HitEndLocation, ECC_Visibility);
			if (HitResult.bBlockingHit)
			{
				ShotgunServerSideRewindResult.bConfirmed = true;
				if (ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(HitResult.GetActor()))
				{
					//根据命中的Hitbox判断是爆头还是身体其他部位被击中，并记录每个角色被击中的次数，后面根据次数来计算伤害
					if (HitCharacter->HitBoxComponentMap.Contains("Head") && HitCharacter->HitBoxComponentMap["Head"] == HitResult.Component)
					{
						AddShotgunHeadShot(ShotgunServerSideRewindResult, HitCharacter);
					}else
					{
						AddShotgunBodyShot(ShotgunServerSideRewindResult, HitCharacter);
					}
				}
			}
		}
	}
	//检测过后，重置所有被改变的属性
	for (auto& OriginPackage : CharacterOriginPackages)
	{
		MoveHitBoxes(OriginPackage, OriginPackage.HitCharacter);
		SetCharacterMeshCollision(OriginPackage.HitCharacter, ECollisionEnabled::Type::QueryAndPhysics);
		SetAllHitBoxCollision(OriginPackage.HitCharacter, ECollisionEnabled::NoCollision);
	}
	return ShotgunServerSideRewindResult;
}

void ULagCompensationComponent::CacheCharacterHitBox(const ABlasterCharacter* HitCharacter, FFramePackage& OutPackage)
{
	if (HitCharacter)
	{
		for (const auto& HitBox : HitCharacter->HitBoxComponentMap)
		{
			FBoxInformation CurrentBoxInfo;
			CurrentBoxInfo.BoxExtent = HitBox.Value->GetScaledBoxExtent();
			CurrentBoxInfo.BoxLocation = HitBox.Value->GetComponentLocation();
			CurrentBoxInfo.BoxRotation = HitBox.Value->GetComponentRotation();
			OutPackage.HitBoxInfo.Add(HitBox.Key, CurrentBoxInfo);
		}
	}
	OutPackage.HitCharacter = const_cast<ABlasterCharacter*>(HitCharacter);
}

void ULagCompensationComponent::MoveHitBoxes(const FFramePackage& InPackage,const ABlasterCharacter* HitCharacter)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxComponent : HitCharacter->HitBoxComponentMap)
	{
		if (HitBoxComponent.Value == nullptr) continue;
		HitBoxComponent.Value->SetBoxExtent(InPackage.HitBoxInfo[HitBoxComponent.Key].BoxExtent);
		HitBoxComponent.Value->SetWorldLocation(InPackage.HitBoxInfo[HitBoxComponent.Key].BoxLocation);
		HitBoxComponent.Value->SetWorldRotation(InPackage.HitBoxInfo[HitBoxComponent.Key].BoxRotation);
	}
}

void ULagCompensationComponent::SetAllHitBoxCollision(const ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxComponent : HitCharacter->HitBoxComponentMap)
	{
		if (HitBoxComponent.Value == nullptr) continue;
		HitBoxComponent.Value->SetCollisionEnabled(CollisionEnabled);
		HitBoxComponent.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

void ULagCompensationComponent::SetHeadHitBoxCollision(const ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter == nullptr) return;
	if (HitCharacter->HitBoxComponentMap.Contains("Head") && HitCharacter->HitBoxComponentMap["Head"])
	{
		HitCharacter->HitBoxComponentMap["Head"]->SetCollisionEnabled(CollisionEnabled);
		HitCharacter->HitBoxComponentMap["Head"]->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

void ULagCompensationComponent::SetCharacterMeshCollision(const ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

void ULagCompensationComponent::Server_ShotgunScoreRequest_Implementation(
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	const TArray<ABlasterCharacter*>& HitCharacters,
	AController* InstigatedBy,
	AWeapon* DamageCauser,
	float HitTime)
{
	FShotgunServerSideRewindResult ShotgunServerSideRewindResult = ShotgunServerRewind(HitTime, TraceStart, HitLocations, HitCharacters);
	if (InstigatedBy && ShotgunServerSideRewindResult.bConfirmed)
	{
		for (auto& HitCharacter : HitCharacters)
		{
			float Damage = DamageCauser->GetDamage();
			float TotalDamage = 0;
			//分别计算每个被击中角色的爆头伤害和身体伤害，最后相加得到总伤害
			if (HitCharacter && ShotgunServerSideRewindResult.CharacterHeadShots.Contains(HitCharacter))
			{
				TotalDamage += ShotgunServerSideRewindResult.CharacterHeadShots[HitCharacter] * Damage;
			}
			if (HitCharacter && ShotgunServerSideRewindResult.CharacterBodyShots.Contains(HitCharacter))
			{
				TotalDamage += ShotgunServerSideRewindResult.CharacterBodyShots[HitCharacter] * Damage;
			}
			if (TotalDamage > 0)
			{
				UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage, InstigatedBy, DamageCauser, UDamageType::StaticClass());
			}
		}
	}
}

void ULagCompensationComponent::Server_ScoreRequest_Implementation(
	const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation,
	ABlasterCharacter* HitCharacter,
	AController* InstigatedBy,
	AWeapon* DamageCauser,
	float HitTime)
{
	FServerSideRewindResult ConfirmResult = ServerRewind(HitTime, TraceStart, HitLocation, HitCharacter);
	if (InstigatedBy && ConfirmResult.bConfirmed)
	{
		float Damage = DamageCauser->GetDamage();
		// Damage = ConfirmResult.bHeadShot ? Damage * 2.f : Damage; //爆头伤害翻倍
		UGameplayStatics::ApplyDamage(HitCharacter, Damage, InstigatedBy, DamageCauser, UDamageType::StaticClass());
	}
}



