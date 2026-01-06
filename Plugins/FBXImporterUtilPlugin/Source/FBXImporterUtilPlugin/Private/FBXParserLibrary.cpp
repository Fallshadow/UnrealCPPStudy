#include "FBXParserLibrary.h"
#include "FBXParserFacade.h"



TArray<FParsedMeshData> UFBXParserLibrary::ParseFBXFile(const FString& FBXFilePath)
{
    FBXParserFacade Facade;
    return Facade.ParseGenericFBX(FBXFilePath);
}

bool UFBXParserLibrary::WriteFBXDataToJSON(const TArray<FParsedMeshData>& ParsedMeshData, const FString& ConfigFileName)
{
    FBXParserFacade Facade;
    return Facade.WriteGenericFBXDataToJSON(ParsedMeshData, ConfigFileName);
}





TArray<FParsedMeshData_FFSLight> UFBXParserLibrary::ParseLightFBX(const FString& FBXFilePath)
{
    FBXParserFacade Facade;
    return Facade.ParseLightFBX(FBXFilePath);
}

bool UFBXParserLibrary::WriteLightFBXDataToJSON(const TArray<FParsedMeshData_FFSLight>& ParsedMeshData, const FString& ConfigFileName)
{
    FBXParserFacade Facade;
    return Facade.WriteLightFBXToJSON(ParsedMeshData, ConfigFileName);
}






TArray<FParsedMeshData_FFSLightModel> UFBXParserLibrary::ParseLightModelFBX(const FString& FBXFilePath)
{
    FBXParserFacade Facade;
    return Facade.ParseLightModelFBX(FBXFilePath);
}

bool UFBXParserLibrary::WriteLightModelFBXDataToJSON(const TArray<FParsedMeshData_FFSLightModel>& ParsedMeshData, const FString& ConfigFileName)
{
    FBXParserFacade Facade;
    return Facade.WriteLightModuleFBXToJSON(ParsedMeshData, ConfigFileName);
}