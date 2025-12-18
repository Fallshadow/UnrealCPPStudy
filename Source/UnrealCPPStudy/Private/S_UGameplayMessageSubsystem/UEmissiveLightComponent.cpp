// Fill out your copyright notice in the Description page of Project Settings.


#include "S_UGameplayMessageSubsystem/UEmissiveLightComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

FGameplayMessageListenerHandle ScoreListenerHandle;

// Sets default values for this component's properties
UUEmissiveLightComponent::UUEmissiveLightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUEmissiveLightComponent::BeginPlay()
{
	Super::BeginPlay();

	static const FGameplayTag ChannelTag = FGameplayTag::RequestGameplayTag(TEXT("AirportLight.Intensity"));
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	ScoreListenerHandle = MessageSubsystem.RegisterListener<FRunWayLightIntensityChangedMessage>(
		ChannelTag,
		this,
		&UUEmissiveLightComponent::OnLightIntensityChanged
	);

	// ...
	
}


// Called every frame
void UUEmissiveLightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUEmissiveLightComponent::OnLightIntensityChanged(FGameplayTag LightIntensityTag, const FRunWayLightIntensityChangedMessage& MessageData) 
{ 
	UE_LOG(LogTemp, Display, TEXT("LightIntensityChange %s %d"), *MessageData.RunWay, MessageData.Intensity);
	
}

