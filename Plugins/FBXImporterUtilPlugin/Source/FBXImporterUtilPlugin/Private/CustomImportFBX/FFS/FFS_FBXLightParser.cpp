// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomImportFBX/FFS/FFS_FBXLightParser.h"

// 解析单个FBX Node的自定义属性
static void ParseFbxNodeProperties(FbxNode* Node, FParsedMeshData_FFSLight& OutProps)
{
    if (!Node) return;

    FbxProperty UserProperties = Node->GetFirstProperty();
    while (UserProperties.IsValid()) {
        FString PropName = UTF8_TO_TCHAR(UserProperties.GetName());
        FbxDataType PropType = UserProperties.GetPropertyDataType();

        if (PropName == TEXT("RwyNum")) {
            FbxString tempFBXStr = UserProperties.Get<FbxString>();
            OutProps.RwyNum = FUTF8ToTCHAR(tempFBXStr.Buffer());
        }
        else if (PropName == TEXT("LightType")) {
            OutProps.LightType = UserProperties.Get<int>();
        }
        else if (PropName == TEXT("VerticalAngle")) {
            OutProps.Angle.X = UserProperties.Get<float>();
        }
        else if (PropName == TEXT("HorizontalAngle")) {
            OutProps.Angle.Y = UserProperties.Get<float>();
        }
        else if (PropName == TEXT("Directional")) {
            OutProps.Directional = UserProperties.Get<int>();
        }
        else if (PropName == TEXT("Freq")) {
            OutProps.Freq = UserProperties.Get<int>();
        }

        // 下一个属性
        UserProperties = Node->GetNextProperty(UserProperties);
    }
}

void FFS_FBXLightParser::ParseFromScene(FbxScene* Scene, TArray<FParsedMeshData_FFSLight>& OutMeshes)
{
    OutMeshes.Reset();
    if (!Scene) return;

    // 递归遍历所有节点
    auto TraverseFBXNodes = [&](FbxNode* Node, auto&& Traverse) -> void
        {
            if (!Node) return;
            FParsedMeshData_FFSLight MeshData = ParseSingleFBXMeshBase(Node);
            if (MeshData.Base.Vertices.Num() > 0) {
                OutMeshes.Add(MeshData);
            }
            for (int32 i = 0; i < Node->GetChildCount(); i++) {
                Traverse(Node->GetChild(i), Traverse);
            }
        };
    TraverseFBXNodes(Scene->GetRootNode(), TraverseFBXNodes);
}

FParsedMeshData_FFSLight FFS_FBXLightParser::ParseSingleFBXMeshBase(FbxNode* FBXNode)
{
    FParsedMeshData_FFSLight ParsedBase;
    if (!FBXNode) return ParsedBase;

    FbxMesh* FBXMesh = FBXNode->GetMesh();
    if (!FBXMesh) {
        UE_LOG(LogTemp, Warning, TEXT("Node %s is not a mesh"), *FString(FBXNode->GetName()));
        return ParsedBase;
    }

    // 网格名称 (FBX SDK 默认 UTF-8/UTF-16，UE FString 有特定编码转换规则)
    ParsedBase.Base.MeshName = FString(UTF8_TO_TCHAR(FBXNode->GetName()));

    // 解析顶点（FBX Y轴向上 → UE Z轴向上）
    // 获取 FBX 网格的 “控制点数组”——FBX 中 “控制点” 就是我们常说的 “顶点”，返回的是 FBX 自定义的四维向量数组（X/Y/Z/W，W 一般是 1.0）；
    FbxVector4* FBXVertices = FBXMesh->GetControlPoints();
    int32 VertexCount = FBXMesh->GetControlPointsCount();
    if (VertexCount <= 0) return ParsedBase;

    // 预分配顶点数组内存（优化性能，避免动态扩容）
    ParsedBase.Base.Vertices.Reserve(VertexCount);
    // 循环遍历每个 FBX 顶点，核心是坐标系转换
    // FBX 坐标系：Y 轴向上（X: 右，Y: 上，Z: 前）；
    // UE 坐标系：Z 轴向上（X: 右，Y: 前，Z: 上）；
    for (int32 v = 0; v < VertexCount; v++) {
        FVector UEVertex(
            FBXVertices[v].mData[0],
            FBXVertices[v].mData[2],
            FBXVertices[v].mData[1]
        );
        ParsedBase.Base.Vertices.Add(UEVertex);
    }

    // 求出具体位置
    int32 CenterPointCount = VertexCount / 3;
    ParsedBase.Base.CenterPoints.Reserve(CenterPointCount);
    for (int32 i = 0; i < CenterPointCount; ++i) {
        int32 idx = i * 3;
        const FVector& P0 = ParsedBase.Base.Vertices[idx];
        const FVector& P1 = ParsedBase.Base.Vertices[idx + 1];
        const FVector& P2 = ParsedBase.Base.Vertices[idx + 2];
        ParsedBase.Base.CenterPoints.Add((P0 + P1 + P2) / 3.0f);
    }

    // 解析三角面（可选，若不需要索引可删除）
    // 获取 FBX 网格的 “面总数”（FBX 中 Polygon 可以是三角面、四边形等）
    int32 PolygonCount = FBXMesh->GetPolygonCount();
    ParsedBase.Base.Triangles.Reserve(PolygonCount * 3);
    for (int32 polyIndex = 0; polyIndex < PolygonCount; polyIndex++) {
        int32 PolygonVertexCount = FBXMesh->GetPolygonSize(polyIndex);
        for (int32 polyVertexIndex = 0; polyVertexIndex < PolygonVertexCount; polyVertexIndex++) {
            ParsedBase.Base.Triangles.Add(FBXMesh->GetPolygonVertex(polyIndex, polyVertexIndex));
        }
    }

    ParseFbxNodeProperties(FBXNode, ParsedBase);

	return ParsedBase;
}

