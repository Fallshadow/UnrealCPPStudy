// Fill out your copyright notice in the Description page of Project Settings.

#include "FBXParseActor.h"

// Sets default values
AFBXParseActor::AFBXParseActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFBXParseActor::BeginPlay()
{
	Super::BeginPlay();

	TArray<FParsedMeshData_FFSLight> meshArray = UFBXParserLibrary::ParseLightFBX(FBXPath);

    // 1. 先输出数组整体信息
    UE_LOG(LogTemp, Log, TEXT("===== 开始遍历FBX解析结果（共%d个网格） ====="), meshArray.Num());

    // 2. 遍历每个网格
    for (int32 MeshIndex = 0; MeshIndex < meshArray.Num(); MeshIndex++) {
        const FParsedMeshData_FFSLight& MeshData = meshArray[MeshIndex]; // 取当前网格数据（const&避免拷贝）

        // 输出当前网格的基础信息
        UE_LOG(LogTemp, Log, TEXT("----- 网格%d：%s -----"), MeshIndex, *MeshData.Base.MeshName);
        UE_LOG(LogTemp, Log, TEXT("顶点数量：%d"), MeshData.Base.Vertices.Num());
        UE_LOG(LogTemp, Log, TEXT("三角面索引数量：%d"), MeshData.Base.Triangles.Num());

        // 3. 遍历当前网格的所有顶点（可选：按需输出，顶点多则注释）
        if (MeshData.Base.Vertices.Num() > 0) {
            UE_LOG(LogTemp, Log, TEXT("【网格%s的顶点数据】"), *MeshData.Base.MeshName);
            for (int32 VertIndex = 0; VertIndex < MeshData.Base.Vertices.Num(); VertIndex++) {
                const FVector& Vertex = MeshData.Base.Vertices[VertIndex];
                UE_LOG(LogTemp, Log, TEXT("顶点%d：X=%.2f, Y=%.2f, Z=%.2f"),
                    VertIndex, Vertex.X, Vertex.Y, Vertex.Z);
            }
        }

        // 4. 遍历当前网格的三角面索引（可选：按需输出，索引多则注释）
        if (MeshData.Base.Triangles.Num() > 0) {
            UE_LOG(LogTemp, Log, TEXT("【网格%s的三角面索引】"), *MeshData.Base.MeshName);
            // 按每3个索引一组（一个三角面）输出，更直观
            for (int32 TriIndex = 0; TriIndex < MeshData.Base.Triangles.Num(); TriIndex += 3) {
                // 防止越界（索引数可能不是3的倍数）
                if (TriIndex + 2 >= MeshData.Base.Triangles.Num()) break;

                int32 Index1 = MeshData.Base.Triangles[TriIndex];
                int32 Index2 = MeshData.Base.Triangles[TriIndex + 1];
                int32 Index3 = MeshData.Base.Triangles[TriIndex + 2];

                UE_LOG(LogTemp, Log, TEXT("三角面%d：%d, %d, %d"),
                    TriIndex / 3, Index1, Index2, Index3);
            }
        }

        MeshData.PrintCustomPropsOnly();
    }

    UE_LOG(LogTemp, Log, TEXT("===== FBX解析结果遍历完成 ====="));

    UFBXParserLibrary::WriteLightFBXDataToJSON(meshArray, ExportFBXJsonName);
}

// Called every frame
void AFBXParseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

