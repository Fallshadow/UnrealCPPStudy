// Copyright Epic Games, Inc. All Rights Reserved.

#include "FBXImporterUtilPlugin.h"

#define LOCTEXT_NAMESPACE "FFBXImporterUtilPluginModule"

void FFBXImporterUtilPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FFBXImporterUtilPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFBXImporterUtilPluginModule, FBXImporterUtilPlugin)