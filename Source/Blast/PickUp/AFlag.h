#pragma once
#include "CoreMinimal.h"
#include "Blast/BlasterType/TeamType.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "AFlag.generated.h"

UENUM(BlueprintType)
enum class EFlagState : uint8
{
	EFS_Initial UMETA(DisplayName = "Initial"),
	EFS_Equipped UMETA(DisplayName = "Equipped"),
	EFS_Dropped UMETA(DisplayName = "Dropped"),

	EFS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLAST_API AFlag : public AActor
{
	GENERATED_BODY()
public:
	AFlag();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int OtherBodyIndex);

	void ShowPickUpWidget(bool bShowPickUpWidget) const;

	void SetFlagState(EFlagState NewState);
	void Dropped();

private:
	void OnEquipped();
	void OnDropped();

	UFUNCTION()
	void OnRep_FlagState();

public:
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* PickUpWidget;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PickUpMesh;
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* OverlapSphere;

private:
	UPROPERTY(ReplicatedUsing = OnRep_FlagState, VisibleAnywhere)
	EFlagState FlagState;

	UPROPERTY(EditDefaultsOnly)
	ETeam TeamType = ETeam::ET_NoTeam;
};
