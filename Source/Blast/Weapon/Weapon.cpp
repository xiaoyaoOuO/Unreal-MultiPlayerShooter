// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Blast/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidget->SetupAttachment(RootComponent);

	WeaponState = EWeaponState::EWS_Initial;

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}

	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this,&AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this,&AWeapon::OnSphereEndOverlap);
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	BlasterPlayerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (BlasterPlayerCharacter)
	{
		BlasterPlayerController = Cast<ABlasterPlayerController>(BlasterPlayerCharacter->Controller);
	}else
	{
		BlasterPlayerController = nullptr;
	}

	UpdateAmmoAmountHUD();
}

void AWeapon::OnRep_AmmoAmount()
{
	UpdateAmmoAmountHUD();
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int OtherBodyIndex)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::UpdateAmmoAmountHUD()
{
	BlasterPlayerCharacter = BlasterPlayerCharacter == nullptr ?  Cast<ABlasterCharacter>(GetOwner()) : BlasterPlayerCharacter;
	if (BlasterPlayerCharacter)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterPlayerCharacter->Controller) : BlasterPlayerController;
		if (BlasterPlayerController)
		{
			BlasterPlayerController->SetBlasterPlayerAmmoAmount(AmmoAmount);
		}
	}
}

void AWeapon::SpendAmmo()
{
	AmmoAmount = FMath::Clamp(AmmoAmount - 1, 0, MagCapacity);

	UpdateAmmoAmountHUD();
}


// Called every frame
void AWeapon::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AWeapon::AddAmmo(int32 Amount)
{
	AmmoAmount = FMath::Clamp(AmmoAmount + Amount, 0, MagCapacity);
}

void AWeapon::ShowPickUpWidget(const bool bShowPickupWidget) const
{
	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowPickupWidget);
	}
}

void AWeapon::SetWeaponState(const EWeaponState State)
{
	WeaponState = State;
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
		{
			//在服务器执行，所以不用判断authority
			ShowPickUpWidget(false);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				break;
		}
		case EWeaponState::EWS_Dropped:
		{
				//客户端和服务器都执行，需要判断authority
			if (HasAuthority())
			{
				AreaSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
			}
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			BlasterPlayerController = nullptr;
			BlasterPlayerCharacter = nullptr;
				break;
		}
		default:
			break;
	}
}

//同步到客户端，修改UI
void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
		{
			ShowPickUpWidget(false);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
				break;
		}
		case EWeaponState::EWS_Dropped:
		{
			// WeaponMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			BlasterPlayerController = nullptr;
			BlasterPlayerCharacter = nullptr;
				break;
		}
		default:
			break;
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, AmmoAmount);
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimationAsset)
	{
		WeaponMesh->PlayAnimation(FireAnimationAsset,false);
	}
	if (BulletShell)
	{
		UE_LOG(LogTemp, Warning,TEXT("create bullet shell"));
		if (const USkeletalMeshSocket* AmmoEject = WeaponMesh->GetSocketByName(FName("AmmoEject")))
		{
			//获取生成子弹的位置（武器mesh存在一个枪口的槽位）
			FTransform MuzzleTransform = AmmoEject->GetSocketTransform(WeaponMesh);
			FRotator Rotation = MuzzleTransform.GetRotation().Rotator();
			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<ACasting>(
					BulletShell, // 子弹的蓝图类
					MuzzleTransform.GetLocation(),
					Rotation
				);
			}
		}
	}
	SpendAmmo();
}

void AWeapon::Dropped()
{
	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentTransformRules);
	SetWeaponState(EWeaponState::EWS_Dropped);
	SetOwner(nullptr);
}

USkeletalMeshComponent* AWeapon::Get_WeaponMesh() const
{
	return WeaponMesh;
}

