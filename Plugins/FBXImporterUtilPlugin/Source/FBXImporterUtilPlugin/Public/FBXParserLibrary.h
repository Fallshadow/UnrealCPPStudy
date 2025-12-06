// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "fbxsdk.h"
#include "FBXParserLibrary.generated.h"

// 结构体：存储解析后的网格数据（顶点 + 索引，方便蓝图访问）
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

    // 插件 TODO: 在下面填写自定义属性

    // 跑道名称（TODO：FBX 中是 string 类型，是否需要变成 int）
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    FString RwyNum;

    // 灯光类型
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    int LightType = 0;

    // VerticalAngle 和 HorizontalAngle
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    FVector2D Angle = FVector2D::ZeroVector;

    // 灯光方向类型
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    int Directional = 0;

    // 闪烁频率
    UPROPERTY(BlueprintReadWrite, Category = "FBX Parser")
    int Freq = 0;

    void PrintCustomPropsOnly() const
    {
        FString CustomStr = FString::Printf(
            TEXT("=== 网格【%s】自定义属性 ===\n")
            TEXT("跑道号：%s | 灯光类型：%d\n")
            TEXT("垂直角度：%.2f | 水平角度：%.2f\n")
            TEXT("灯光方向类型：%d | 闪烁频率：%d"),
            *MeshName,
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

UCLASS()
class FBXIMPORTERUTILPLUGIN_API UFBXParserLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "FBX Parser", meta = (DisplayName = "Parse FBX File"))
    static TArray<FParsedMeshData> ParseFBXFile(const FString& FBXFilePath);

    UFUNCTION(BlueprintCallable, Category = "FBX Parser", meta = (DisplayName = "Write FBX Data to JSON"))
    static bool WriteFBXDataToJSON(const TArray<FParsedMeshData>& ParsedMeshData, const FString& ConfigFileName = TEXT("FBX_Config"));

private:
	static  FbxManager* CreateFBXManager();
	static  void DestroyFBXManager();
	// 全局静态变量：存储复用的 FBX 管理器（防止重复调用创建多个实例）
	static FbxManager* GFBXManagerInstance;

	// 从单个 FBX 节点中提取网格数据（顶点、三角面索引），并将 FBX 坐标系转换为 UE 坐标系，最终返回封装好的网格数据结构体
    static FParsedMeshData ParseSingleFBXMeshes(FbxNode* FBXNode);

    // 辅助函数：创建保存目录（若不存在）
    static bool CreateSaveDirectory(FString& OutSavePath);
};
