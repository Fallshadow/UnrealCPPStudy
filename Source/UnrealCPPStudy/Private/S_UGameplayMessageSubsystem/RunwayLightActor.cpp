// Fill out your copyright notice in the Description page of Project Settings.


#include "S_UGameplayMessageSubsystem/RunwayLightActor.h"
#include "S_UGameplayMessageSubsystem/GameMessage.h"
#include "GameFramework/GameplayMessageSubsystem.h"

// Sets default values
ARunwayLightActor::ARunwayLightActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARunwayLightActor::SetIntensity(int32 value)
{
	//// 使用 GameplayMessageSubsystem 广播
	//UGameplayMessageSubsystem::Get(this).BroadcastMessage(
	//	TEXT("Game.Player.ScoreChanged"), // 频道名，可以改成你喜欢的
	//	Msg
	//);

	FRunWayLightIntensityChangedMessage rwLightIntensityM;
	rwLightIntensityM.RunWay = "15";
	rwLightIntensityM.Intensity = value;

	static const FGameplayTag rwLightIntensityChangeTag = FGameplayTag::RequestGameplayTag(TEXT("Game.RWLightIntensityChange"));

	UGameplayMessageSubsystem::Get(this).BroadcastMessage
		<FRunWayLightIntensityChangedMessage>(rwLightIntensityChangeTag, rwLightIntensityM);
}

// Called when the game starts or when spawned
void ARunwayLightActor::BeginPlay()
{
	Super::BeginPlay();
	

	SetIntensity(1);
}

// Called every frame
void ARunwayLightActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

