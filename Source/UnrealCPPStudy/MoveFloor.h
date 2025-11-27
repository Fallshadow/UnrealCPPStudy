// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MoveFloor.generated.h"

UCLASS()
class UNREALCPPSTUDY_API AMoveFloor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMoveFloor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	FVector Velocity;
	UPROPERTY(EditAnywhere)
	FRotator RotatorVelocity;
	UPROPERTY(EditAnywhere)
	float TargetDistance = 300.0f;
private:
	FVector FloorPosTemp;
	FVector FloorPosStart;
	FVector FloorPosEnd;

	float nowDistance;


	void MoveFloor(float DeltaTime);
	void RotateFloor(float DeltaTime);
};
