// Fill out your copyright notice in the Description page of Project Settings.

#include "CustomImportFBX/FFS/FFS_FBXLightModelParser.h"

// 解析单个FBX Node的自定义属性
static void ParseFbxNodeProperties(FbxNode* Node, FParsedMeshData_FFSLightModel& OutProps)
{
    if (!Node) return;

    FbxProperty UserProperties = Node->GetFirstProperty();
    while (UserProperties.IsValid()) {
        FString PropName = UTF8_TO_TCHAR(UserProperties.GetName());
        FbxDataType PropType = UserProperties.GetPropertyDataType();

        

        // 下一个属性
        UserProperties = Node->GetNextProperty(UserProperties);
    }
}

void FFS_FBXLightModelParser::ParseFromScene(FbxScene* Scene, TArray<FParsedMeshData_FFSLightModel>& OutMeshes)
{
    OutMeshes.Reset();
    if (!Scene) return;

    // 递归遍历所有节点
    auto TraverseFBXNodes = [&](FbxNode* Node, auto&& Traverse) -> void
        {
            if (!Node) return;
            FParsedMeshData_FFSLightModel MeshData = ParseSingleFBXMeshBase(Node);
            if (MeshData.Base.Vertices.Num() > 0) {
                OutMeshes.Add(MeshData);
            }
            for (int32 i = 0; i < Node->GetChildCount(); i++) {
                Traverse(Node->GetChild(i), Traverse);
            }
        };

    TraverseFBXNodes(Scene->GetRootNode(), TraverseFBXNodes);
}

static void ExtractUVLayer(FbxMesh* FBXMesh, int32 UVLayerIndex, TArray<FVector2D>& OutUVs)
{
    if (!FBXMesh) return;

    FbxLayerElementUV* UVElem = FBXMesh->GetElementUV(UVLayerIndex);
    if (!UVElem) {
        // 某些模型可能没有这么多层，直接跳过即可
        return;
    }

    const int32 PolygonCount = FBXMesh->GetPolygonCount();
    if (PolygonCount <= 0) {
        return;
    }

    const auto MappingMode = UVElem->GetMappingMode();
    const auto RefMode = UVElem->GetReferenceMode();

    const FbxLayerElementArrayTemplate<FbxVector2>& DirectArray = UVElem->GetDirectArray();
    const FbxLayerElementArrayTemplate<int>& IndexArray = UVElem->GetIndexArray();

    OutUVs.Reset();
    // 这里是四边形
    OutUVs.Reserve(PolygonCount);

    auto GetUVByIndex = [&](int32 UVIndex) -> FVector2D
        {
            if (UVIndex < 0 || UVIndex >= DirectArray.GetCount()) {
                return FVector2D::ZeroVector;
            }
            const FbxVector2& UV = DirectArray.GetAt(UVIndex);
            return FVector2D(
                static_cast<float>(UV[0]),
                static_cast<float>(UV[1])
            );
        };

    int32 GlobalUVIndex = 0; // 专供 eByPolygonVertex 使用

    for (int32 PolyIdx = 0; PolyIdx < PolygonCount; ++PolyIdx) {
        const int32 VertCount = FBXMesh->GetPolygonSize(PolyIdx);

        GlobalUVIndex = PolyIdx * 4;
        
        int32 UVIndex = 0;

        if (MappingMode == FbxLayerElement::eByPolygonVertex) {
            if (RefMode == FbxLayerElement::eDirect) {
                UVIndex = GlobalUVIndex;
            }
            else if (RefMode == FbxLayerElement::eIndexToDirect) {
                if (GlobalUVIndex >= 0 && GlobalUVIndex < IndexArray.GetCount()) {
                    UVIndex = IndexArray.GetAt(GlobalUVIndex);
                }
            }
        }
        else if (MappingMode == FbxLayerElement::eByControlPoint) {
            const int32 CtrlPointIndex = FBXMesh->GetPolygonVertex(PolyIdx, 0);

            if (RefMode == FbxLayerElement::eDirect) {
                UVIndex = CtrlPointIndex;
            }
            else if (RefMode == FbxLayerElement::eIndexToDirect) {
                if (CtrlPointIndex >= 0 && CtrlPointIndex < IndexArray.GetCount()) {
                    UVIndex = IndexArray.GetAt(CtrlPointIndex);
                }
            }
        }
        else {
            // 其他 MappingMode 不处理
            OutUVs.Add(FVector2D::ZeroVector);
            continue;
        }

        OutUVs.Add(GetUVByIndex(UVIndex));
    }
}

FParsedMeshData_FFSLightModel FFS_FBXLightModelParser::ParseSingleFBXMeshBase(FbxNode* FBXNode)
{
    FParsedMeshData_FFSLightModel ParsedBase;
    if (!FBXNode) return ParsedBase;

    FbxMesh* FBXMesh = FBXNode->GetMesh();
    if (!FBXMesh) {
        UE_LOG(LogTemp, Warning, TEXT("Node %s is not a mesh"), *FString(FBXNode->GetName()));
        return ParsedBase;
    }

    // 网格名称 (FBX SDK 默认 UTF-8/UTF-16，UE FString 有特定编码转换规则)
    ParsedBase.Base.MeshName = FString(UTF8_TO_TCHAR(FBXNode->GetName()));

    // FBX 里可以有多个 Layer，每个 Layer 里可以有一个 VertexColor
    FbxLayer* Layer0 = FBXMesh->GetLayer(0);
    if (!Layer0) {
        UE_LOG(LogTemp, Warning, TEXT("Mesh %s has no layers"), *FString(UTF8_TO_TCHAR(FBXMesh->GetName())));
        return ParsedBase;
    }

    FbxLayerElementVertexColor* VertexColorElem = Layer0->GetVertexColors();
    if (!VertexColorElem) {
        UE_LOG(LogTemp, Warning, TEXT("Mesh %s has no vertex colors (LayerElementColor)"), *FString(UTF8_TO_TCHAR(FBXMesh->GetName())));
        return ParsedBase;
    }

    FbxLayerElement::EMappingMode MappingMode = VertexColorElem->GetMappingMode();
    FbxLayerElement::EReferenceMode RefMode = VertexColorElem->GetReferenceMode();

    const FbxLayerElementArrayTemplate<FbxColor>& ColorDirectArray = VertexColorElem->GetDirectArray();
    const FbxLayerElementArrayTemplate<int>& ColorIndexArray = VertexColorElem->GetIndexArray();

    const int32 PolygonCount = FBXMesh->GetPolygonCount();

    ParsedBase.VertexColors.Reset();
    ParsedBase.VertexColors.Reserve(PolygonCount); // 仅四边形


    auto GetColorByIndex = [&](int32 ColorIndex) -> FLinearColor
        {
            if (ColorIndex < 0 || ColorIndex >= ColorDirectArray.GetCount()) {
                return FLinearColor::White;
            }
            const FbxColor& C = ColorDirectArray.GetAt(ColorIndex);
            return FLinearColor(
                static_cast<float>(C.mRed),
                static_cast<float>(C.mGreen),
                static_cast<float>(C.mBlue),
                static_cast<float>(C.mAlpha)
            );
        };

    auto GetColor = [&](int32 PolyIdx, int32 VertInPoly) -> FLinearColor
        {
            int32 GlobalVertexIndex = PolyIdx * 4 + VertInPoly;

            int32 ColorIndex = 0;

            switch (MappingMode) {
                case FbxLayerElement::eByControlPoint:
                {
                    int32 CtrlPointIndex = FBXMesh->GetPolygonVertex(PolyIdx, VertInPoly);
                    if (RefMode == FbxLayerElement::eDirect) {
                        ColorIndex = CtrlPointIndex;
                    }
                    else if (RefMode == FbxLayerElement::eIndexToDirect) {
                        if (CtrlPointIndex >= 0 && CtrlPointIndex < ColorIndexArray.GetCount()) {
                            ColorIndex = ColorIndexArray.GetAt(CtrlPointIndex);
                        }
                    }
                }
                break;

                case FbxLayerElement::eByPolygonVertex:
                {
                    if (RefMode == FbxLayerElement::eDirect) {
                        ColorIndex = GlobalVertexIndex;
                    }
                    else if (RefMode == FbxLayerElement::eIndexToDirect) {
                        if (GlobalVertexIndex >= 0 && GlobalVertexIndex < ColorIndexArray.GetCount()) {
                            ColorIndex = ColorIndexArray.GetAt(GlobalVertexIndex);
                        }
                    }
                }
                break;

                default:
                    return FLinearColor::White;
            }

            return GetColorByIndex(ColorIndex);
        };

    // 都是四边形，选择第一个点的信息存储就好
    for (int32 PolyIdx = 0; PolyIdx < PolygonCount; ++PolyIdx) {
        const int32 VertCount = FBXMesh->GetPolygonSize(PolyIdx);
        FLinearColor Col = GetColor(PolyIdx, 0);
        ParsedBase.VertexColors.Add(Col);
    }

    ExtractUVLayer(FBXMesh, 0, ParsedBase.UVs0);
    ExtractUVLayer(FBXMesh, 1, ParsedBase.UVs1);
    ExtractUVLayer(FBXMesh, 2, ParsedBase.UVs2);

    ParseFbxNodeProperties(FBXNode, ParsedBase);

    return ParsedBase;
}

