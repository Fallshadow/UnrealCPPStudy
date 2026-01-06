#include "FBXParserFacade.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

#include "CustomImportFBX/FFS/FFS_FBXLightParser.h"
#include "CustomImportFBX/FFS/FFS_FBXLightModelParser.h"
#include "CustomImportFBX/CustomFBXParser.h"

FbxManager* FBXParserFacade::GFBXManagerInstance = nullptr;

FBXParserFacade::FBXParserFacade() { }

FBXParserFacade::~FBXParserFacade()
{
}

#pragma region FBXManager

FbxManager* FBXParserFacade::GetOrCreateManager()
{
    // 先检查是否已创建，已创建则直接返回
    if (GFBXManagerInstance) return GFBXManagerInstance;

    // 创建 FBX 管理器核心实例
    GFBXManagerInstance = FbxManager::Create();
    if (!GFBXManagerInstance) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create FBX Manager!"));
        return nullptr;
    }

    // 检查 FbxIOSettings 创建结果（避免空指针传入 SetIOSettings）
    FbxIOSettings* IOSettings = FbxIOSettings::Create(GFBXManagerInstance, IOSROOT);
    if (!IOSettings) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create FBX IO Settings!"));
        // IO配置创建失败，需销毁已创建的管理器，避免内存泄漏
        GFBXManagerInstance->Destroy();
        GFBXManagerInstance = nullptr;
        return nullptr;
    }

    GFBXManagerInstance->SetIOSettings(IOSettings);
    return GFBXManagerInstance;
}

void FBXParserFacade::DestroyManager()
{
    if (GFBXManagerInstance) {
        // FBX SDK 父子对象机制：销毁管理器会自动销毁其创建的 IOSettings
        GFBXManagerInstance->Destroy();
        // 重置全局变量，避免野指针
        GFBXManagerInstance = nullptr;
        UE_LOG(LogTemp, Log, TEXT("FBX Manager destroyed successfully!"));
    }
}

bool FBXParserFacade::InitFBX(const FString& FBXFilePath, FbxScene*& OutScene, FbxImporter*& OutImporter)
{
    OutScene = nullptr;
    OutImporter = nullptr;

    if (!FPaths::FileExists(FBXFilePath)) {
        UE_LOG(LogTemp, Error, TEXT("FBX File not found: %s"), *FBXFilePath);
        return false;
    }

    FbxManager* Manager = GetOrCreateManager();
    if (!Manager) return false;

    OutImporter = FbxImporter::Create(Manager, "CustomFBXImporter");
    if (!OutImporter->Initialize(TCHAR_TO_UTF8(*FBXFilePath), -1, Manager->GetIOSettings())) {
        UE_LOG(LogTemp, Error, TEXT("FBX Import Failed: %s"), UTF8_TO_TCHAR(OutImporter->GetStatus().GetErrorString()));
        OutImporter->Destroy();
        OutImporter = nullptr;
        return false;
    }

    OutScene = FbxScene::Create(Manager, "ImportedFBXScene");
    if (!OutImporter->Import(OutScene)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to import FBX into scene"));
        OutImporter->Destroy();
        OutScene->Destroy();
        OutImporter = nullptr;
        OutScene = nullptr;
        return false;
    }

    return true;
}

void FBXParserFacade::DestroyFBX(FbxScene* Scene, FbxImporter* Importer)
{
    // 释放资源
    // 按 “创建逆序” 销毁 FBX 对象：导入器 → 场景 → IO 配置 → 管理器；
    // IO 配置 和 管理器全局，最后再销毁
    if (Importer) Importer->Destroy();
    if (Scene) Scene->Destroy();
}

bool FBXParserFacade::CanParsed(const FString& FBXFilePath, FbxScene*& OutScene, FbxImporter*& OutImporter)
{
    if (!FPaths::FileExists(FBXFilePath)) {
        UE_LOG(LogTemp, Error, TEXT("FBX File not found:%s"), *FBXFilePath);
        return false;
    }

    GetOrCreateManager();
    if (!GFBXManagerInstance) return false;

    if (!InitFBX(FBXFilePath, OutScene, OutImporter)) return false;

    return true;
}

#pragma endregion

#pragma region Json

bool FBXParserFacade::WriteGenericFBXDataToJSON(const TArray<FParsedMeshData>& ParsedMeshData, const FString& ConfigFileName)
{
    if (ParsedMeshData.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("No parsed mesh data to write!"));
        return false;
    }

    // 项目 /Saved/FBXVertexConfigs/
    FString SaveDir = FPaths::ProjectSavedDir() + TEXT("FBXDataConfigs/");
    if (!FPaths::DirectoryExists(SaveDir)) {
        if (!IFileManager::Get().MakeDirectory(*SaveDir, true)) {
            UE_LOG(LogTemp, Error, TEXT("Failed to create save directory: %s"), *SaveDir);
            return false;
        }
    }

    FString FinalFilePath = SaveDir + ConfigFileName + TEXT(".json");

    // 构建 JSON 数据结构
    TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
    // 添加文件信息（可选，方便识别）
    RootObj->SetNumberField(TEXT("Mesh_Count"), ParsedMeshData.Num());
    RootObj->SetStringField(TEXT("Export_Time"), FDateTime::Now().ToString());

    // 遍历每个网格，添加顶点数据
    TArray<TSharedPtr<FJsonValue>> MeshArray;
    for (const FParsedMeshData& MeshData : ParsedMeshData) {
        TSharedPtr<FJsonObject> MeshObj = MakeShareable(new FJsonObject());
        MeshObj->SetStringField(TEXT("Mesh_Name"), MeshData.MeshName);  // 网格名称
        MeshObj->SetNumberField(TEXT("Vertex_Count"), MeshData.Vertices.Num());  // 顶点数量

        // 格式化顶点数据：将 FVector 转为 "X,Y,Z" 字符串数组
        TArray<TSharedPtr<FJsonValue>> VertexArray;
        for (const FVector& Vertex : MeshData.Vertices) {
            FString VertexStr = FString::Printf(TEXT("%.10g,%.10g,%.10g"), Vertex.X, Vertex.Y, Vertex.Z);
            VertexArray.Add(MakeShareable(new FJsonValueString(VertexStr)));
        }
        MeshObj->SetArrayField(TEXT("Vertices"), VertexArray);

        // 格式化中心点数据：
        TArray<TSharedPtr<FJsonValue>> CenterArray;
        for (const FVector& Center : MeshData.CenterPoints) {
            FString CenterStr = FString::Printf(TEXT("%.10g,%.10g,%.10g"), Center.X, Center.Y, Center.Z);
            CenterArray.Add(MakeShareable(new FJsonValueString(CenterStr)));
        }
        MeshObj->SetArrayField(TEXT("Center_Vertices"), CenterArray);

        // （可选）添加三角面索引数据
        TArray<TSharedPtr<FJsonValue>> TriangleArray;
        for (int32 TriangleIdx : MeshData.Triangles) {
            TriangleArray.Add(MakeShareable(new FJsonValueNumber(TriangleIdx)));
        }
        MeshObj->SetArrayField(TEXT("Triangles"), TriangleArray);

        //// 自定义字段
        //MeshObj->SetStringField(TEXT("RwyNum"), MeshData.RwyNum);
        //MeshObj->SetNumberField(TEXT("LightType"), MeshData.LightType);
        //MeshObj->SetNumberField(TEXT("VerticalAngle"), MeshData.Angle.X);
        //MeshObj->SetNumberField(TEXT("HorizontalAngle"), MeshData.Angle.Y);
        //MeshObj->SetNumberField(TEXT("Directional"), MeshData.Directional);
        //MeshObj->SetNumberField(TEXT("Freq"), MeshData.Freq);

        MeshArray.Add(MakeShareable(new FJsonValueObject(MeshObj)));
    }
    RootObj->SetArrayField(TEXT("Meshes"), MeshArray);

    // 将 JSON 转为字符串并写入文件
    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    if (!FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON data!"));
        return false;
    }

    if (!FFileHelper::SaveStringToFile(JsonStr, *FinalFilePath)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to write JSON file: %s"), *FinalFilePath);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully wrote vertex data to JSON: %s"), *FinalFilePath);
    return true;
}

bool FBXParserFacade::WriteLightFBXToJSON(const TArray<FParsedMeshData_FFSLight>& ParsedMeshData, const FString& ConfigFileName)
{
    if (ParsedMeshData.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("No parsed mesh data to write!"));
        return false;
    }

    // 项目 /Saved/FBXVertexConfigs/
    FString SaveDir = FPaths::ProjectSavedDir() + TEXT("FBXDataConfigs/");
    if (!FPaths::DirectoryExists(SaveDir)) {
        if (!IFileManager::Get().MakeDirectory(*SaveDir, true)) {
            UE_LOG(LogTemp, Error, TEXT("Failed to create save directory: %s"), *SaveDir);
            return false;
        }
    }

    FString FinalFilePath = SaveDir + ConfigFileName + TEXT(".json");

    // 构建 JSON 数据结构
    TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());
    // 添加文件信息（可选，方便识别）
    RootObj->SetNumberField(TEXT("Mesh_Count"), ParsedMeshData.Num());
    RootObj->SetStringField(TEXT("Export_Time"), FDateTime::Now().ToString());

    // 遍历每个网格，添加顶点数据
    TArray<TSharedPtr<FJsonValue>> MeshArray;
    for (const FParsedMeshData_FFSLight& MeshData : ParsedMeshData) {
        TSharedPtr<FJsonObject> MeshObj = MakeShareable(new FJsonObject());
        MeshObj->SetStringField(TEXT("Mesh_Name"), MeshData.Base.MeshName);  // 网格名称
        MeshObj->SetNumberField(TEXT("Vertex_Count"), MeshData.Base.Vertices.Num());  // 顶点数量

        // 格式化顶点数据：将 FVector 转为 "X,Y,Z" 字符串数组
        TArray<TSharedPtr<FJsonValue>> VertexArray;
        for (const FVector& Vertex : MeshData.Base.Vertices) {
            FString VertexStr = FString::Printf(TEXT("%.10g,%.10g,%.10g"), Vertex.X, Vertex.Y, Vertex.Z);
            VertexArray.Add(MakeShareable(new FJsonValueString(VertexStr)));
        }
        MeshObj->SetArrayField(TEXT("Vertices"), VertexArray);

        // 格式化中心点数据：
        TArray<TSharedPtr<FJsonValue>> CenterArray;
        for (const FVector& Center : MeshData.Base.CenterPoints) {
            FString CenterStr = FString::Printf(TEXT("%.10g,%.10g,%.10g"), Center.X, Center.Y, Center.Z);
            CenterArray.Add(MakeShareable(new FJsonValueString(CenterStr)));
        }
        MeshObj->SetArrayField(TEXT("Center_Vertices"), CenterArray);

        // （可选）添加三角面索引数据
        TArray<TSharedPtr<FJsonValue>> TriangleArray;
        for (int32 TriangleIdx : MeshData.Base.Triangles) {
            TriangleArray.Add(MakeShareable(new FJsonValueNumber(TriangleIdx)));
        }
        MeshObj->SetArrayField(TEXT("Triangles"), TriangleArray);

        // 自定义字段
        MeshObj->SetStringField(TEXT("RwyNum"), MeshData.RwyNum);
        MeshObj->SetNumberField(TEXT("LightType"), MeshData.LightType);
        MeshObj->SetNumberField(TEXT("VerticalAngle"), MeshData.Angle.X);
        MeshObj->SetNumberField(TEXT("HorizontalAngle"), MeshData.Angle.Y);
        MeshObj->SetNumberField(TEXT("Directional"), MeshData.Directional);
        MeshObj->SetNumberField(TEXT("Freq"), MeshData.Freq);

        MeshArray.Add(MakeShareable(new FJsonValueObject(MeshObj)));
    }
    RootObj->SetArrayField(TEXT("Meshes"), MeshArray);

    // 将 JSON 转为字符串并写入文件
    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    if (!FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON data!"));
        return false;
    }

    if (!FFileHelper::SaveStringToFile(JsonStr, *FinalFilePath)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to write JSON file: %s"), *FinalFilePath);
        return false;
    }

    UE_LOG(LogTemp, Log, TEXT("Successfully wrote vertex data to JSON: %s"), *FinalFilePath);
    return true;
    return true;
}

bool FBXParserFacade::WriteLightModuleFBXToJSON(const TArray<FParsedMeshData_FFSLightModel>& ParsedMeshData, const FString& ConfigFileName)
{
    return false;
}

#pragma endregion

TArray<FParsedMeshData> FBXParserFacade::ParseGenericFBX(const FString& FBXFilePath)
{
    TArray<FParsedMeshData> Result;
    FbxScene* Scene = nullptr;
    FbxImporter* Importer = nullptr;
    if (!CanParsed(FBXFilePath, Scene, Importer)) return Result;

    CustomFBXParser* Parser = new CustomFBXParser();
    Parser->ParseFromScene(Scene, Result);
    delete Parser;

    DestroyFBX(Scene, Importer);
    return Result;
}



TArray<FParsedMeshData_FFSLight> FBXParserFacade::ParseLightFBX(const FString& FBXFilePath)
{
    TArray<FParsedMeshData_FFSLight> Result;
    FbxScene* Scene = nullptr;
    FbxImporter* Importer = nullptr;
    if (!CanParsed(FBXFilePath, Scene, Importer)) return Result;

    FFS_FBXLightParser* Parser = new FFS_FBXLightParser();
    Parser->ParseFromScene(Scene, Result);
    delete Parser;

    DestroyFBX(Scene, Importer);
    return Result;
}

TArray<FParsedMeshData_FFSLightModel> FBXParserFacade::ParseLightModelFBX(const FString& FBXFilePath)
{
    TArray<FParsedMeshData_FFSLightModel> Result;
    FbxScene* Scene = nullptr;
    FbxImporter* Importer = nullptr;
    if (!CanParsed(FBXFilePath, Scene, Importer)) return Result;

    FFS_FBXLightModelParser* Parser = new FFS_FBXLightModelParser();
    Parser->ParseFromScene(Scene, Result);
    delete Parser;

    DestroyFBX(Scene, Importer);
    return Result;
}