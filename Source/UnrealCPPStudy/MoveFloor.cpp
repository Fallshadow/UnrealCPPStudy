// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveFloor.h"

// Sets default values
AMoveFloor::AMoveFloor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMoveFloor::BeginPlay()
{
	Super::BeginPlay();

	FloorPosStart = GetActorLocation();
}

// Called every frame
void AMoveFloor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	nowDistance = FVector::Dist(FloorPosStart, FloorPosEnd);
	MoveFloor(DeltaTime);
	if (nowDistance >= TargetDistance) {

		FVector dir = Velocity.GetSafeNormal();
		FloorPosStart = FloorPosStart + dir * TargetDistance;
		SetActorLocation(FloorPosStart);

		UE_LOG(LogTemp, Display, TEXT("Now Set %s : %f %f %f"), *(GetName()), FloorPosStart.X, FloorPosStart.Y, FloorPosStart.Z);
		
		Velocity = -Velocity;
	}
}

void AMoveFloor::MoveFloor(float DeltaTime)
{ 
	FloorPosEnd = GetActorLocation();
	FloorPosEnd = FloorPosEnd + Velocity * DeltaTime;
	SetActorLocation(FloorPosEnd);
}
