// CustomFBXParser.h
#pragma once

#include "CoreMinimal.h"
#include "../../FBXParsedTypes.h"
#include "fbxsdk.h"


class FFS_FBXLightParser {
public:
    // 把整个 Scene 解析成基础网格数组
    void ParseFromScene(FbxScene* Scene, TArray<FParsedMeshData_FFSLight>& OutMeshes);

private:
    // 单个 Node → 基础网格数据
    FParsedMeshData_FFSLight ParseSingleFBXMeshBase(FbxNode* FBXNode);
};