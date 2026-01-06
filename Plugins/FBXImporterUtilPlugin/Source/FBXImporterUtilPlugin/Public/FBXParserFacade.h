// FBXParserFacade.h

#pragma once

#include "CoreMinimal.h"
#include "FBXParsedTypes.h"
// FBX SDK
#include "fbxsdk.h"

class FBXIMPORTERUTILPLUGIN_API FBXParserFacade {
public:
    FBXParserFacade();
    ~FBXParserFacade();

    // 通用解析——内部可选默认解析器
    TArray<FParsedMeshData> ParseGenericFBX(const FString& FBXFilePath);

    // 针对灯光 FBX
    TArray<FParsedMeshData_FFSLight> ParseLightFBX(const FString& FBXFilePath);

    // 针对灯光模型 FBX
    TArray<FParsedMeshData_FFSLightModel> ParseLightModelFBX(const FString& FBXFilePath);

    bool WriteGenericFBXDataToJSON(const TArray<FParsedMeshData>& ParsedMeshData, const FString& ConfigFileName);
    bool WriteLightFBXToJSON(const TArray<FParsedMeshData_FFSLight>& ParsedMeshData, const FString& ConfigFileName);
    bool WriteLightModuleFBXToJSON(const TArray<FParsedMeshData_FFSLightModel>& ParsedMeshData, const FString& ConfigFileName);

private:
    bool CanParsed(const FString& FBXFilePath, FbxScene*& OutScene, FbxImporter*& OutImporter);
    bool InitFBX(const FString& FBXFilePath, FbxScene*& OutScene, class FbxImporter*& OutImporter);
    void DestroyFBX(FbxScene* Scene, class FbxImporter* Importer);

    FbxManager* GetOrCreateManager();
    void DestroyManager();

    static FbxManager* GFBXManagerInstance;  // 由 Facade 统一管理
};