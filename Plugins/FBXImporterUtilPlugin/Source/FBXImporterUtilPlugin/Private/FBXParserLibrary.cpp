#include "FBXParserLibrary.h"

// 路径和文件操作功能
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

// JSON 序列化支持
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

// 全局静态变量：存储复用的 FBX 管理器（防止重复调用创建多个实例）
FbxManager* UFBXParserLibrary::GFBXManagerInstance = nullptr;

FbxManager* UFBXParserLibrary::CreateFBXManager() {
    // 1. 解决重复调用问题：先检查是否已创建，已创建则直接返回
    if (GFBXManagerInstance) {
        return GFBXManagerInstance;
    }

    // 创建 FBX 管理器核心实例
    GFBXManagerInstance = FbxManager::Create();
    if (!GFBXManagerInstance) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create FBX Manager!"));
        return nullptr;
    }

    // 2. 检查 FbxIOSettings 创建结果（避免空指针传入 SetIOSettings）
    FbxIOSettings* IOSettings = FbxIOSettings::Create(GFBXManagerInstance, IOSROOT);
    if (!IOSettings) {
        UE_LOG(LogTemp, Error, TEXT("Failed to create FBX IO Settings!"));
        // IO配置创建失败，需销毁已创建的管理器，避免内存泄漏
        GFBXManagerInstance->Destroy();
        return nullptr;
    }

    GFBXManagerInstance->SetIOSettings(IOSettings);

    // 将创建成功的管理器赋值给全局变量，供后续复用
    GFBXManagerInstance = GFBXManagerInstance;
    return GFBXManagerInstance;
}

void UFBXParserLibrary::DestroyFBXManager()
{
    if (GFBXManagerInstance) {
        // FBX SDK 父子对象机制：销毁管理器会自动销毁其创建的 IOSettings
        GFBXManagerInstance->Destroy();
        // 重置全局变量，避免野指针
        GFBXManagerInstance = nullptr;
        UE_LOG(LogTemp, Log, TEXT("FBX Manager destroyed successfully!"));
    }
}

// 辅助函数：创建保存目录（项目/Saved/FBXVertexConfigs/）
bool UFBXParserLibrary::CreateSaveDirectory(FString& OutSavePath)
{
    // 配置文件保存路径：项目/Saved/FBXVertexConfigs/
    OutSavePath = FPaths::ProjectSavedDir() + TEXT("FBXDataConfigs/");

    // 若目录不存在，创建目录
    if (!FPaths::DirectoryExists(OutSavePath)) {
        bool bCreated = IFileManager::Get().MakeDirectory(*OutSavePath, true);
        if (!bCreated) {
            UE_LOG(LogTemp, Error, TEXT("Failed to create save directory: %s"), *OutSavePath);
            return false;
        }
        UE_LOG(LogTemp, Log, TEXT("Created save directory: %s"), *OutSavePath);
    }
    return true;
}

// 解析单个FBX Node的自定义属性
static void ParseFbxNodeProperties(FbxNode* Node, FParsedMeshData& OutProps)
{
    if (!Node) return;

    FbxProperty UserProperties = Node->GetFirstProperty();
    while (UserProperties.IsValid()) {
        FString PropName = UTF8_TO_TCHAR(UserProperties.GetName());
        FbxDataType PropType = UserProperties.GetPropertyDataType();

        // 插件 TODO：在下面解析自定义属性
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

TArray<FParsedMeshData> UFBXParserLibrary::ParseFBXFile(const FString& FBXFilePath)
{
    TArray<FParsedMeshData> parsedMeshDatas;
    if (!FPaths::FileExists(FBXFilePath)) {
        UE_LOG(LogTemp, Error, TEXT("FBX File not found:%s"), *FBXFilePath);
        return parsedMeshDatas;
    }

    CreateFBXManager();
    if (!GFBXManagerInstance) return parsedMeshDatas;

    FbxImporter* FBXImporter = FbxImporter::Create(GFBXManagerInstance, "CustomFBXImporter");
    // 将 UE 的 FString（TCHAR 编码）转为 UTF8 编码（FBX SDK 要求的编码格式）；
    if (!FBXImporter->Initialize(TCHAR_TO_UTF8(*FBXFilePath), -1, GFBXManagerInstance->GetIOSettings())) {
        UE_LOG(LogTemp, Error, TEXT("FBX Import Failed: %s"), UTF8_TO_TCHAR(FBXImporter->GetStatus().GetErrorString()));
        FBXImporter->Destroy();
        return parsedMeshDatas;
    }

    FbxScene* FBXScene = FbxScene::Create(GFBXManagerInstance, "ImportedFBXScene");
    if (!FBXImporter->Import(FBXScene)) {
        UE_LOG(LogTemp, Error, TEXT("Failed to import FBX into scene"));
        FBXImporter->Destroy();
        FBXScene->Destroy();
        return parsedMeshDatas;
    }

    // 递归遍历所有节点
    auto TraverseFBXNodes = [&](FbxNode* Node, auto&& Traverse) -> void
        {
            if (!Node) return;
            FParsedMeshData MeshData = ParseSingleFBXMeshes(Node);
            if (MeshData.Vertices.Num() > 0) {
                parsedMeshDatas.Add(MeshData);
            }
            for (int32 i = 0; i < Node->GetChildCount(); i++) {
                Traverse(Node->GetChild(i), Traverse);
            }
        };
    TraverseFBXNodes(FBXScene->GetRootNode(), TraverseFBXNodes);

    // 释放资源
    // 按 “创建逆序” 销毁 FBX 对象：导入器 → 场景 → IO 配置 → 管理器；
    FBXImporter->Destroy();
    FBXScene->Destroy();

    // 输出日志：记录解析完成，以及提取到的网格总数；
    UE_LOG(LogTemp, Display, TEXT("FBX Parse Completed! Total meshes: %d"), parsedMeshDatas.Num());

    return parsedMeshDatas;
}

FParsedMeshData UFBXParserLibrary::ParseSingleFBXMeshes(FbxNode* FBXNode) {
    FParsedMeshData ParsedData;
    if (!FBXNode) return ParsedData;

    FbxMesh* FBXMesh = FBXNode->GetMesh();
    if (!FBXMesh) {
        UE_LOG(LogTemp, Warning, TEXT("Node %s is not a mesh"), *FString(FBXNode->GetName()));
        return ParsedData;
    }

    // 网格名称 (FBX SDK 默认 UTF-8/UTF-16，UE FString 有特定编码转换规则)
    ParsedData.MeshName = FString(UTF8_TO_TCHAR(FBXNode->GetName()));

    // 解析顶点（FBX Y轴向上 → UE Z轴向上）
    // 获取 FBX 网格的 “控制点数组”——FBX 中 “控制点” 就是我们常说的 “顶点”，返回的是 FBX 自定义的四维向量数组（X/Y/Z/W，W 一般是 1.0）；
    FbxVector4* FBXVertices = FBXMesh->GetControlPoints();
    int32 VertexCount = FBXMesh->GetControlPointsCount();
    if (VertexCount <= 0) {
        UE_LOG(LogTemp, Warning, TEXT("Mesh %s has no vertices!"), *ParsedData.MeshName);
        return ParsedData;
    }

    // 预分配顶点数组内存（优化性能，避免动态扩容）
    ParsedData.Vertices.Reserve(VertexCount);
    // 循环遍历每个 FBX 顶点，核心是坐标系转换
    // FBX 坐标系：Y 轴向上（X: 右，Y: 上，Z: 前）；
    // UE 坐标系：Z 轴向上（X: 右，Y: 前，Z: 上）；
    // END：这里先不改变顺序试试  其实就还是需要调换
    for (int32 v = 0; v < VertexCount; v++) {
        FVector UEVertex(
            FBXVertices[v].mData[0],
            FBXVertices[v].mData[2],
            FBXVertices[v].mData[1]
            //FBXVertices[v].mData[1],
            //FBXVertices[v].mData[2]
        );
        ParsedData.Vertices.Add(UEVertex);
    }

    // 求出具体位置
    int centerPointCount = VertexCount / 3;
    ParsedData.CenterPoints.Reserve(centerPointCount);
    for (int32 pdv = 0; pdv < centerPointCount; pdv++) {
        int verticesIndex = pdv * 3;

        FVector P0 = ParsedData.Vertices[verticesIndex];
        FVector P1 = ParsedData.Vertices[verticesIndex + 1];
        FVector P2 = ParsedData.Vertices[verticesIndex + 2];
        FVector Center = (P0 + P1 + P2) / 3.0;
        ParsedData.CenterPoints.Add(Center);
    }

    // 解析三角面（可选，若不需要索引可删除）
    // 获取 FBX 网格的 “面总数”（FBX 中 Polygon 可以是三角面、四边形等）
    int32 PolygonCount = FBXMesh->GetPolygonCount();
    ParsedData.Triangles.Reserve(PolygonCount * 3);
    for (int32 polyIndex = 0; polyIndex < PolygonCount; polyIndex++) {
        int32 PolygonVertexCount = FBXMesh->GetPolygonSize(polyIndex);
        for (int32 polyVertexIndex = 0; polyVertexIndex < PolygonVertexCount; polyVertexIndex++) {
            ParsedData.Triangles.Add(FBXMesh->GetPolygonVertex(polyIndex, polyVertexIndex));
        }
    }

    ParseFbxNodeProperties(FBXNode, ParsedData);

    UE_LOG(LogTemp, Log, TEXT("Parsed mesh: %s | Vertices: %d"), *ParsedData.MeshName, ParsedData.Vertices.Num());
    return ParsedData;
}

// 写入 JSON 配置文件
bool UFBXParserLibrary::WriteFBXDataToJSON(const TArray<FParsedMeshData>& ParsedMeshData, const FString& ConfigFileName)
{
    // 1. 检查输入数据是否有效
    if (ParsedMeshData.Num() == 0) {
        UE_LOG(LogTemp, Error, TEXT("No parsed mesh data to write!"));
        return false;
    }

    // 2. 创建保存目录，获取最终文件路径
    FString SaveDir;
    if (!CreateSaveDirectory(SaveDir)) {
        return false;
    }
    FString FinalFilePath = SaveDir + ConfigFileName + TEXT(".json");  // 完整路径：Saved/FBXVertexConfigs/xxx.json

    // 3. 构建 JSON 数据结构
    TSharedPtr<FJsonObject> RootObj = MakeShareable(new FJsonObject());

    // 3.1 添加文件信息（可选，方便识别）
    RootObj->SetNumberField(TEXT("Mesh_Count"), ParsedMeshData.Num());
    RootObj->SetStringField(TEXT("Export_Time"), FDateTime::Now().ToString());  // 导出时间

    // 3.2 遍历每个网格，添加顶点数据
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
        TArray<TSharedPtr<FJsonValue>> CenterPointArray;
        for (const FVector& Vertex : MeshData.CenterPoints) {
            FString VertexStr = FString::Printf(TEXT("%.10g,%.10g,%.10g"), Vertex.X, Vertex.Y, Vertex.Z);
            CenterPointArray.Add(MakeShareable(new FJsonValueString(VertexStr)));
        }
        MeshObj->SetArrayField(TEXT("Center_Vertices"), CenterPointArray);

        // （可选）添加三角面索引数据
        TArray<TSharedPtr<FJsonValue>> TriangleArray;
        for (int32 TriangleIdx : MeshData.Triangles) {
            TriangleArray.Add(MakeShareable(new FJsonValueNumber(TriangleIdx)));
        }
        MeshObj->SetArrayField(TEXT("Triangles"), TriangleArray);

        // TODO: 自定义属性解析
        MeshObj->SetStringField(TEXT("RwyNum"), MeshData.RwyNum);
        MeshObj->SetNumberField(TEXT("LightType"), MeshData.LightType);
        MeshObj->SetNumberField(TEXT("VerticalAngle"), MeshData.Angle.X);
        MeshObj->SetNumberField(TEXT("HorizontalAngle"), MeshData.Angle.Y);
        MeshObj->SetNumberField(TEXT("Directional"), MeshData.Directional);
        MeshObj->SetNumberField(TEXT("Freq"), MeshData.Freq);

        MeshArray.Add(MakeShareable(new FJsonValueObject(MeshObj)));
    }
    RootObj->SetArrayField(TEXT("Meshes"), MeshArray);

    // 4. 将 JSON 转为字符串并写入文件
    FString JsonStr;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonStr, 0);  // 0 = 无缩进，4 = 带缩进（可读性强）
    if (FJsonSerializer::Serialize(RootObj.ToSharedRef(), JsonWriter)) {
        // 写入文件（覆盖已存在的文件）
        bool bWriteSuccess = FFileHelper::SaveStringToFile(JsonStr, *FinalFilePath);
        if (bWriteSuccess) {
            UE_LOG(LogTemp, Log, TEXT("Successfully wrote vertex data to JSON: %s"), *FinalFilePath);
            return true;
        }
        else {
            UE_LOG(LogTemp, Error, TEXT("Failed to write JSON file: %s"), *FinalFilePath);
            return false;
        }
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON data!"));
        return false;
    }
}