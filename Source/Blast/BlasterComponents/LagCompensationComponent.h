// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blast/PlayerController/BlasterPlayerController.h"
#include "Components/ActorComponent.h"
#include "Blast/Weapon/Weapon.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector BoxLocation;

	UPROPERTY()
	FRotator BoxRotation;
	
	UPROPERTY()
	FVector BoxExtent;
};

/**
 * @param Time 记录帧数据的时间戳
 * @param HitBoxInfo 记录每个HitBox的位置、旋转和大小
 */
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	FFramePackage()
		: Time(0.f), HitCharacter(nullptr)
	{
	}

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlasterCharacter* HitCharacter;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT()
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bConfirmed;

	UPROPERTY()
	TMap<ABlasterCharacter*, uint8> CharacterHeadShots;

	UPROPERTY()
	TMap<ABlasterCharacter*, uint8> CharacterBodyShots;
};


/**
 * 在服务端存储历史帧数据，并在客户端请求回溯时，服务端根据请求的时间戳找到对应的历史帧数据，移动被击中角色的HitBox到历史位置进行碰撞检测，最后重置HitBox位置
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLAST_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ABlasterCharacter;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	ABlasterPlayerController* Controller;

	//保存历史帧数据的链表,头部是最新的帧数据，尾部是最旧的帧数据
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f; //记录历史帧的最长时间
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SaveFramePackage(FFramePackage& Package);
	void ShowFramePackage(const FFramePackage& Package);
	void AddFrame();
	void UpdateFrameHistory();
	/**
	* @param HitTime 被击中时的服务器时间
	* @param TraceStart 射线的起点
	* @param HitLocation 被击中的位置
	* @param HitCharacter 被击中的角色
	*/
	FServerSideRewindResult ServerRewind(float HitTime, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const ABlasterCharacter* HitCharacter);

	/**
	* @param HitTime 被击中时的服务器时间
	* @param TraceStart 射线的起点
	* @param HitLocations 散弹枪所有的被击中位置
	* @param HitCharacters 所有可能被击中的角色
	*/
	FShotgunServerSideRewindResult ShotgunServerRewind(float HitTime, const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocations,const TArray<ABlasterCharacter*>& HitCharacters);

	/**
	* @param HitTime 被击中时的服务器时间
	* @param TraceStart 射线的起点
	* @param InitialVelocity 初始速度
	* @param HitCharacter 被击中的角色
	*/
	FServerSideRewindResult ProjectileServerRewind(float HitTime, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, const ABlasterCharacter* HitCharacter);
	
	FFramePackage FrameInterp(const FFramePackage& OldFrame,const FFramePackage& YoungFrame,float HitTime);

	//根据HitTime找到对应的历史帧数据，如果没有完全匹配的帧数据，就插值计算出HitTime时的帧数据
	FFramePackage GetFrameToCheck(float HitTime, const ABlasterCharacter* HitCharacter);

	//及时命中确认，客户端和服务端同时进行碰撞检测，服务端根据客户端传来的HitLocation和TraceStart进行碰撞检测，如果服务端也检测到命中，就确认命中有效，否则无效
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package,const FVector_NetQuantize& TraceStart,const FVector_NetQuantize& HitLocation,const ABlasterCharacter* HitCharacter);
	
	//散弹枪命中确认，客户端和服务端同时进行碰撞检测，服务端根据客户端传来的HitLocations和TraceStart进行碰撞检测，如果服务端也检测到命中，就确认命中有效，否则无效
	FShotgunServerSideRewindResult ShotgunConfirmHit(TArray<FFramePackage>& Package,const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocations);

	//子弹命中确认，客户端和服务端同时进行碰撞检测，服务端根据客户端传来的HitLocation和TraceStart进行碰撞检测，如果服务端也检测到命中，就确认命中有效，否则无效
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package,const FVector_NetQuantize& TraceStart,const FVector_NetQuantize100& InitialVelocity,const ABlasterCharacter* HitCharacter);

	void AddShotgunHeadShot(FShotgunServerSideRewindResult& ShotgunServerSideRewindResult,
	                        ABlasterCharacter* HitCharacter);
	
	void AddShotgunBodyShot(FShotgunServerSideRewindResult& ShotgunServerSideRewindResult,
	                        ABlasterCharacter* HitCharacter);

	//存储当前的Hitbox位置，用于后面的碰撞检测以及ResetCharacter的Hitbox位置
	void CacheCharacterHitBox(const ABlasterCharacter* HitCharacter, FFramePackage& OutPackage);

	//移动Character的Hitbox，用于后面的碰撞检测以及ResetCharacter的Hitbox位置
	void MoveHitBoxes(const FFramePackage& InPackage,const ABlasterCharacter* HitCharacter);

	void SetAllHitBoxCollision(const ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	void SetHeadHitBoxCollision(const ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	void SetCharacterMeshCollision(const ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(Server, Reliable)
	void Server_ScoreRequest(const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, ABlasterCharacter* HitCharacter, AController* InstigatedBy, AWeapon* DamageCauser, float HitTime);

	UFUNCTION(Server, Reliable)
	void Server_ShotgunScoreRequest(const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocations,const TArray<ABlasterCharacter*>& HitCharacters, AController* InstigatedBy, AWeapon* DamageCauser, float HitTime);
};
