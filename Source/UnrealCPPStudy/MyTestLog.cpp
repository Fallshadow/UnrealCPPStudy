// Fill out your copyright notice in the Description page of Project Settings.


#include "MyTestLog.h"

// Sets default values
AMyTestLog::AMyTestLog()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMyTestLog::BeginPlay()
{
	Super::BeginPlay();
	
	FString myName = "SunShuchao";
	UE_LOG(LogTemp, Display, TEXT("My Name : %s"), *myName);
}

// Called every frame
void AMyTestLog::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

