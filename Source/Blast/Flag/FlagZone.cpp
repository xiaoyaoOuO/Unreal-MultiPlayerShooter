// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "Blast/Character/BlasterCharacter.h"
#include "Blast/GameMode/TeamCaptureFlagGameMode.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	ZoneSphere->SetupAttachment(RootComponent);
	ZoneSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	ZoneSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECR_Overlap);

	DecoratedMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DecoratedMesh"));
	DecoratedMesh->SetupAttachment(ZoneSphere);
	DecoratedMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	DecoratedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ZoneSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		ZoneSphere->OnComponentBeginOverlap.AddDynamic(this,&AFlagZone::OnSphereOverlap);
	}
}

void AFlagZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* Character = Cast<ABlasterCharacter>(OtherActor);
	if (Character && Character->Get_Team() == TeamTag && Character->IsHoldingFlag())
	{
		if (ATeamCaptureFlagGameMode* GameMode = Cast<ATeamCaptureFlagGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->FlagCaptured(Character);
		}
	}
}

