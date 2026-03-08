#include "AFlag.h"

#include "Blast/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Logging/LogMacros.h"
#include "Net/UnrealNetwork.h"


AFlag::AFlag()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpMesh"));
	// PickUpMesh->SetupAttachment(RootComponent);
	SetRootComponent(PickUpMesh);

	PickUpMesh->SetCollisionResponseToAllChannels(ECR_Block);
	PickUpMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	PickUpMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickUpMesh->SetRenderCustomDepth(false);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidget->SetupAttachment(RootComponent);

	FlagState = EFlagState::EFS_Initial;
}

void AFlag::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		if (TeamType == BlasterCharacter->Get_Team() && !BlasterCharacter->IsHoldingFlag())
		{
			BlasterCharacter->SetOverlappingFlag(this);
		}else
		{
			UE_LOG(LogTemp,Error,TEXT("BlasterCharacter is on the same team or already holding a flag, cannot pick up this flag."));
		}
	}
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	ShowPickUpWidget(false);
	
	if (OverlapSphere && OverlapSphere->GetAttachParent() != PickUpMesh)
	{
		if (OverlapSphere->GetAttachParent() == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("RootParent of OverlapSphere is None, expected PickUpMesh. Reattaching at runtime."));
		}else
		{
			UE_LOG(LogTemp, Error, TEXT("RootParent of OverlapSphere is %s, expected PickUpMesh. Reattaching at runtime."), *OverlapSphere->GetAttachParent()->GetName());
		}
	}

	if (PickUpWidget && PickUpWidget->GetAttachParent() != PickUpMesh)
	{
		if (OverlapSphere->GetAttachParent() == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("RootParent of OverlapSphere is None, expected PickUpMesh. Reattaching at runtime."));
		}else
		{
			UE_LOG(LogTemp, Error, TEXT("RootParent of PickUpWidget is %s, expected PickUpMesh. Reattaching at runtime."), *PickUpWidget->GetAttachParent()->GetName());
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Flag BeginPlay CompState | MeshAbs(L:%d R:%d S:%d) WidgetAbs(L:%d R:%d S:%d) MeshParent:%s WidgetParent:%s"),
		PickUpMesh ? PickUpMesh->IsUsingAbsoluteLocation() : -1,
		PickUpMesh ? PickUpMesh->IsUsingAbsoluteRotation() : -1,
		PickUpMesh ? PickUpMesh->IsUsingAbsoluteScale() : -1,
		PickUpWidget ? PickUpWidget->IsUsingAbsoluteLocation() : -1,
		PickUpWidget ? PickUpWidget->IsUsingAbsoluteRotation() : -1,
		PickUpWidget ? PickUpWidget->IsUsingAbsoluteScale() : -1,
		PickUpMesh && PickUpMesh->GetAttachParent() ? *PickUpMesh->GetAttachParent()->GetName() : TEXT("None"),
		PickUpWidget && PickUpWidget->GetAttachParent() ? *PickUpWidget->GetAttachParent()->GetName() : TEXT("None"));

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlag::OnSphereOverlap);
	OverlapSphere->OnComponentEndOverlap.AddDynamic(this, &AFlag::OnSphereEndOverlap);

	InitialTransform = GetActorTransform();
}

void AFlag::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AFlag::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComponent, int OtherBodyIndex)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		BlasterCharacter->SetOverlappingFlag(nullptr);
	}
}

void AFlag::ShowPickUpWidget(bool bShowPickUpWidget) const
{
	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(bShowPickUpWidget);
	}
}

void AFlag::OnEquipped()
{
	ShowPickUpWidget(false);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	PickUpMesh->SetEnableGravity(false);
	PickUpMesh->SetSimulatePhysics(false);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AFlag::OnDropped()
{
	if (HasAuthority())
	{
		OverlapSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	}
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	PickUpMesh->SetSimulatePhysics(true);
	PickUpMesh->SetEnableGravity(true);
}

void AFlag::SetFlagState(EFlagState NewState)
{
	FlagState = NewState;
	switch (FlagState)
	{
	case EFlagState::EFS_Equipped:
		OnEquipped();
		break;
	case EFlagState::EFS_Dropped:
		OnDropped();
		break;
	default:
		break;
	}
}

void AFlag::OnRep_FlagState()
{
	switch (FlagState)
	{
	case EFlagState::EFS_Equipped:
		OnEquipped();
		break;
	case EFlagState::EFS_Dropped:
		OnDropped();
		break;
	default:
		break;
	}
}

void AFlag::Dropped(bool bResetLocation)
{
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	PickUpMesh->DetachFromComponent(DetachRules);
	SetFlagState(EFlagState::EFS_Dropped);
	SetOwner(nullptr);
	if (bResetLocation && HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(ResetFlagTimer, this, &AFlag::ResetFlagLocation, 2.f);
	}
}

void AFlag::ResetFlagLocation()
{
	SetActorTransform(InitialTransform);
}

void AFlag::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFlag, FlagState);
}
