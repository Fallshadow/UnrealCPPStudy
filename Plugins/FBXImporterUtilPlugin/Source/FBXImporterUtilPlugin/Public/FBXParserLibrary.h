// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "fbxsdk.h"
#include "FBXParsedTypes.h"
#include "FBXParserLibrary.generated.h"

UCLASS()
class FBXIMPORTERUTILPLUGIN_API UFBXParserLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    // 通用 FBX 解析
    UFUNCTION(BlueprintCallable, Category = "FBX Parser", meta = (DisplayName = "Parse FBX File"))
    static TArray<FParsedMeshData> ParseFBXFile(const FString& FBXFilePath);

    UFUNCTION(BlueprintCallable, Category = "FBX Parser", meta = (DisplayName = "Write FBX Data to JSON"))
    static bool WriteFBXDataToJSON(const TArray<FParsedMeshData>& ParsedMeshData, const FString& ConfigFileName = TEXT("FBX_Config"));

    // 灯光专用解析
    UFUNCTION(BlueprintCallable, Category = "FBX Parser|Light")
    static TArray<FParsedMeshData_FFSLight> ParseLightFBX(const FString& FBXFilePath);

    UFUNCTION(BlueprintCallable, Category = "FBX Parser|Light", meta = (DisplayName = "Write FBX Light Data to JSON"))
    static bool WriteLightFBXDataToJSON(const TArray<FParsedMeshData_FFSLight>& ParsedMeshData, const FString& ConfigFileName = TEXT("FBX_Config"));

    // 灯光模型专用解析
    UFUNCTION(BlueprintCallable, Category = "FBX Parser|LightModel")
    static TArray<FParsedMeshData_FFSLightModel> ParseLightModelFBX(const FString& FBXFilePath);

    UFUNCTION(BlueprintCallable, Category = "FBX Parser|LightModel", meta = (DisplayName = "Write FBX LightModel Data to JSON"))
    static bool WriteLightModelFBXDataToJSON(const TArray<FParsedMeshData_FFSLightModel>& ParsedMeshData, const FString& ConfigFileName = TEXT("FBX_Config"));
};
