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

	TArray<FParsedMeshData> meshArray = UFBXParserLibrary::ParseFBXFile(FBXPath);

    // 1. 先输出数组整体信息
    UE_LOG(LogTemp, Log, TEXT("===== 开始遍历FBX解析结果（共%d个网格） ====="), meshArray.Num());

    // 2. 遍历每个网格
    for (int32 MeshIndex = 0; MeshIndex < meshArray.Num(); MeshIndex++) {
        const FParsedMeshData& MeshData = meshArray[MeshIndex]; // 取当前网格数据（const&避免拷贝）

        // 输出当前网格的基础信息
        UE_LOG(LogTemp, Log, TEXT("----- 网格%d：%s -----"), MeshIndex, *MeshData.MeshName);
        UE_LOG(LogTemp, Log, TEXT("顶点数量：%d"), MeshData.Vertices.Num());
        UE_LOG(LogTemp, Log, TEXT("三角面索引数量：%d"), MeshData.Triangles.Num());

        // 3. 遍历当前网格的所有顶点（可选：按需输出，顶点多则注释）
        if (MeshData.Vertices.Num() > 0) {
            UE_LOG(LogTemp, Log, TEXT("【网格%s的顶点数据】"), *MeshData.MeshName);
            for (int32 VertIndex = 0; VertIndex < MeshData.Vertices.Num(); VertIndex++) {
                const FVector& Vertex = MeshData.Vertices[VertIndex];
                UE_LOG(LogTemp, Log, TEXT("顶点%d：X=%.2f, Y=%.2f, Z=%.2f"),
                    VertIndex, Vertex.X, Vertex.Y, Vertex.Z);
            }
        }

        // 4. 遍历当前网格的三角面索引（可选：按需输出，索引多则注释）
        if (MeshData.Triangles.Num() > 0) {
            UE_LOG(LogTemp, Log, TEXT("【网格%s的三角面索引】"), *MeshData.MeshName);
            // 按每3个索引一组（一个三角面）输出，更直观
            for (int32 TriIndex = 0; TriIndex < MeshData.Triangles.Num(); TriIndex += 3) {
                // 防止越界（索引数可能不是3的倍数）
                if (TriIndex + 2 >= MeshData.Triangles.Num()) break;

                int32 Index1 = MeshData.Triangles[TriIndex];
                int32 Index2 = MeshData.Triangles[TriIndex + 1];
                int32 Index3 = MeshData.Triangles[TriIndex + 2];

                UE_LOG(LogTemp, Log, TEXT("三角面%d：%d, %d, %d"),
                    TriIndex / 3, Index1, Index2, Index3);
            }
        }

        MeshData.PrintCustomPropsOnly();
    }

    UE_LOG(LogTemp, Log, TEXT("===== FBX解析结果遍历完成 ====="));

    UFBXParserLibrary::WriteFBXDataToJSON(meshArray, ExportFBXJsonName);
}

// Called every frame
void AFBXParseActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

