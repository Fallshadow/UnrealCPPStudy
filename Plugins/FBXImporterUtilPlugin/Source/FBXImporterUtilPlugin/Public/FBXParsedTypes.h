// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FBXParsedTypes.generated.h"


// ======================
// 基础网格数据（所有 FBX 通用）
// ======================
USTRUCT(BlueprintType)
struct FParsedMeshData {
    GENERATED_BODY()

    // 顶点数组（UE 坐标系：Z 轴向上）
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector> Vertices;

    // 位置数组（每个 Mesh 的中心点，作为新生成面片的位置）
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector> CenterPoints;

    // 三角面索引数组（每 3 个元素组成一个三角面）
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<int32> Triangles;

    // 网格名称（FBX 中的节点名称）
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    FString MeshName;
};


// ======================
// 灯光 FBX 专用的结构
// ======================
USTRUCT(BlueprintType)
struct FParsedMeshData_FFSLight {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    FParsedMeshData Base;

    // 跑道名称（TODO：FBX 中是 string 类型，是否需要变成 int）
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser|FFS|Light")
    FString RwyNum;

    // 灯光类型
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser|FFS|Light")
    int32 LightType = 0;

    // VerticalAngle 和 HorizontalAngle
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser|FFS|Light")
    FVector2D Angle = FVector2D::ZeroVector;

    // 灯光方向类型
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser|FFS|Light")
    int32 Directional = 0;

    // 闪烁频率
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser|FFS|Light")
    int32 Freq = 0;

    void PrintCustomPropsOnly() const
    {
        FString CustomStr = FString::Printf(
            TEXT("=== 网格【%s】自定义属性 ===\n")
            TEXT("跑道号：%s | 灯光类型：%d\n")
            TEXT("垂直角度：%.2f | 水平角度：%.2f\n")
            TEXT("灯光方向类型：%d | 闪烁频率：%d"),
            *Base.MeshName,
            *RwyNum,
            LightType,
            Angle.Y,
            Angle.X,
            Directional,
            Freq
        );
        UE_LOG(LogTemp, Warning, TEXT("%s"), *CustomStr); // Warning级别，日志中更醒目
    }
};


// ======================
// 灯光模型 FBX 专用的结构
// ======================
USTRUCT(BlueprintType)
struct FParsedMeshData_FFSLightModel {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    FParsedMeshData Base;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector> Scale;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector> Rotation;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FLinearColor> VertexColors;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector2D> UVs0;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector2D> UVs1;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector2D> UVs2;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FVector> OutPositions;

    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    TArray<FQuat>   OutRotations;

    // 变体编号
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser|FFS|Light")
    TArray<int> Variants;
};