// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"

#include "Blast/Weapon/WeaponType.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
APickUp::APickUp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);

	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpMesh"));
	PickUpMesh->SetupAttachment(OverlapSphere);
	PickUpMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	PickUpMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	PickUpMesh->SetRenderCustomDepth(true);

	PickUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealthComponent"));
	PickUpNiagaraComponent->SetupAttachment(RootComponent);
}

void APickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickUpMesh)
	{
		PickUpMesh->AddLocalRotation(FRotator(0, 45.f * DeltaTime, 0));
	}
}

// Called when the game starts or when spawned
void APickUp::BeginPlay()
{
	Super::BeginPlay();

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this,&APickUp::OnSphereOverlap);
}

void APickUp::Destroyed()
{
	Super::Destroyed();

	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),PickUpSound,GetActorLocation());
	}

	if (PickUpEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickUpEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}




