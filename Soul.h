// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Soul.generated.h"

/**
 * 
 */
UCLASS()
class PROYECTO4_API ASoul : public AItem
{
	GENERATED_BODY()
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	FTimerHandle PickupTimerHandle;
	bool bCanPickup = false;
	
public:
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InitialPickupDelay = 0.7f;

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnEmmiter(AActor* OtherActor);
protected:
	virtual void BeginPlay() override;
	
private:

	UPROPERTY(EditAnywhere, Category = "Soul Properties")
	int32 Souls;

	double DesiredZ;

	UPROPERTY(EditAnywhere)
	float DriftRate = -15.f;
	void EnablePickup();
public:
	FORCEINLINE int32 GetSouls() const { return Souls; }
	FORCEINLINE void SetSouls(int32 NumberOfSouls) { Souls = NumberOfSouls; }
};
