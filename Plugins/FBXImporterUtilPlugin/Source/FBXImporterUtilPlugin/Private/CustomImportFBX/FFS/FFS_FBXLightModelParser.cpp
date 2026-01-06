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

    const FbxLayerElementArrayTemplate<FbxColor>& DirectArray = VertexColorElem->GetDirectArray();
    const FbxLayerElementArrayTemplate<int>& IndexArray = VertexColorElem->GetIndexArray();

    const int32 PolygonCount = FBXMesh->GetPolygonCount();

    TArray<FLinearColor> VertexColors;
    VertexColors.Reset();
    VertexColors.Reserve(PolygonCount * 4); // 仅四边形


    auto GetColorByIndex = [&](int32 ColorIndex) -> FLinearColor
        {
            if (ColorIndex < 0 || ColorIndex >= DirectArray.GetCount()) {
                return FLinearColor::White;
            }
            const FbxColor& C = DirectArray.GetAt(ColorIndex);
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
                        if (CtrlPointIndex >= 0 && CtrlPointIndex < IndexArray.GetCount()) {
                            ColorIndex = IndexArray.GetAt(CtrlPointIndex);
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
                        if (GlobalVertexIndex >= 0 && GlobalVertexIndex < IndexArray.GetCount()) {
                            ColorIndex = IndexArray.GetAt(GlobalVertexIndex);
                        }
                    }
                }
                break;

                default:
                    return FLinearColor::White;
            }

            return GetColorByIndex(ColorIndex);
        };

    // 遍历每个面、每个多边形顶点
    for (int32 PolyIdx = 0; PolyIdx < PolygonCount; ++PolyIdx) {
        const int32 VertCount = FBXMesh->GetPolygonSize(PolyIdx);
        for (int32 VertInPoly = 0; VertInPoly < VertCount; ++VertInPoly) {
            FLinearColor Col = GetColor(PolyIdx, VertInPoly);
            VertexColors.Add(Col);
        }
    }

    ParseFbxNodeProperties(FBXNode, ParsedBase);

    return ParsedBase;
}

