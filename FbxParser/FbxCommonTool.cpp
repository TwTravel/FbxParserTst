#include "FbxCommonTool.h"
#include <iostream>
#include <fstream>
#include <fbxsdk/scene/fbxaxissystem.h>

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pFbxManager->GetIOSettings()))
#endif

FbxCommonTool* FbxCommonTool::fbxCommonTool = nullptr;

FbxCommonTool::FbxCommonTool()
{
}

FbxCommonTool::~FbxCommonTool()
{
}

FbxCommonTool* FbxCommonTool::GetInstance()
{
	if (fbxCommonTool == nullptr)
		fbxCommonTool = new FbxCommonTool();
	return fbxCommonTool;
}

// 初始化fbx manager
void FbxCommonTool::InitializeSdkObjects(FbxManager*& pFbxManager, FbxScene*& pFbxScene)
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	pFbxManager = FbxManager::Create();
	if (!pFbxManager)
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pFbxManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pFbxManager, IOSROOT);
	pFbxManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pFbxManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pFbxScene = FbxScene::Create(pFbxManager, "My Scene");
	if (!pFbxScene)
	{
		FBXSDK_printf("Error: Unable to create FBX scene!\n");
		exit(1);
	}
}

// 销毁fbx manager
void FbxCommonTool::DestroySdkObjects(FbxManager* pFbxManager, bool pExitStatus)
{
	//Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
	if (pFbxManager) 
		pFbxManager->Destroy();
	if (pExitStatus) 
		FBXSDK_printf("Program Success!\n");
}

// 保持场景到fbx文件中
bool FbxCommonTool::SaveScene(FbxManager* pFbxManager, FbxDocument* pFbxScene, const char* pFilename, int pFileFormat/* = -1*/, bool pEmbedMedia/* = false*/)
{
	int lMajor, lMinor, lRevision;
	bool lStatus = true;

	// Create an exporter.
	FbxExporter* lExporter = FbxExporter::Create(pFbxManager, "");

	if (pFileFormat < 0 || pFileFormat >= pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount())
	{
		// Write in fall back format in less no ASCII format found
		pFileFormat = pFbxManager->GetIOPluginRegistry()->GetNativeWriterFormat();

		//Try to export in ASCII if possible
		int lFormatIndex, lFormatCount = pFbxManager->GetIOPluginRegistry()->GetWriterFormatCount();

		for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (pFbxManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = pFbxManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char *lASCII = "ascii";
				if (lDesc.Find(lASCII) >= 0)
				{
					pFileFormat = lFormatIndex;
					break;
				}
			}
		}
	}

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
	// shows how to change these states.
	IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
	IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
	IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, pEmbedMedia);
	IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
	IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
	IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
	IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	// Initialize the exporter by providing a filename.
	if (lExporter->Initialize(pFilename, pFileFormat, pFbxManager->GetIOSettings()) == false)
	{
		FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		return false;
	}

	FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
	FBXSDK_printf("FBX file format version %d.%d.%d\n\n", lMajor, lMinor, lRevision);

	// Export the scene.
	lStatus = lExporter->Export(pFbxScene);

	// Destroy the exporter.
	lExporter->Destroy();

	return lStatus;
}

// 导入fbx文件到场景中
bool FbxCommonTool::LoadScene(FbxManager* pFbxManager, FbxDocument* pFbxScene, const char* pFilename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor, lSDKMinor, lSDKRevision;
	//int lFileFormat = -1;
	int i, lAnimStackCount;
	bool lStatus;
	char lPassword[1024];

	// Get the file version number generate by the FBX SDK.
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(pFbxManager, "");

	// Initialize the importer by providing a filename.
	const bool lImportStatus = lImporter->Initialize(pFilename, -1, pFbxManager->GetIOSettings());
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if (!lImportStatus)
	{
		FbxString error = lImporter->GetStatus().GetErrorString();
		FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
			FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
		}

		return false;
	}

	FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

	if (lImporter->IsFBX())
	{
		FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

		// From this point, it is possible to access animation stack information without
		// the expense of loading the entire file.

		FBXSDK_printf("Animation Stack Information\n");

		lAnimStackCount = lImporter->GetAnimStackCount();

		FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
		FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
		FBXSDK_printf("\n");

		for (i = 0; i < lAnimStackCount; i++)
		{
			FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

			FBXSDK_printf("    Animation Stack %d\n", i);
			FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
			FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

			// Change the value of the import name if the animation stack should be imported 
			// under a different name.
			FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

			// Set the value of the import state to false if the animation stack should be not
			// be imported. 
			FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
			FBXSDK_printf("\n");
		}

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
		IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	// Import the scene.
	lStatus = lImporter->Import(pFbxScene);

	if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
	{
		FBXSDK_printf("Please enter password: ");

		lPassword[0] = '\0';

		FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
			scanf("%s", lPassword);
		FBXSDK_CRT_SECURE_NO_WARNING_END

			FbxString lString(lPassword);

		IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
		IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

		lStatus = lImporter->Import(pFbxScene);

		if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			FBXSDK_printf("\nPassword is wrong, import aborted.\n");
		}
	}

	// Destroy the importer.
	lImporter->Destroy();

	return lStatus;
}

// 将场景转换成3d max场景
void FbxCommonTool::CovertSceneTo3DMax(FbxScene* pFbxScene)
{
	int dir;
	FbxAxisSystem::EUpVector lUpVector = pFbxScene->GetGlobalSettings().GetAxisSystem().GetUpVector(dir); // this returns the equivalent of FbxAxisSystem::eYAxis
	FBXSDK_printf("current UP vector is: %d\n", lUpVector);
	FbxAxisSystem max(FbxAxisSystem::EPreDefinedAxisSystem::eMax); // we desire to convert the scene from Y-Up to Z-Up
	max.ConvertScene(pFbxScene);
	lUpVector = pFbxScene->GetGlobalSettings().GetAxisSystem().GetUpVector(dir); // this will now return the equivalent of FbxAxisSystem::eZAxis
	FBXSDK_printf("after scene covert, UP vector is: %d\n", lUpVector);
}

// 将场景转换成opengl 场景
void FbxCommonTool::CovertSceneToOpenGL(FbxScene* pFbxScene)
{
	int dir;
	FbxAxisSystem::EUpVector lUpVector = pFbxScene->GetGlobalSettings().GetAxisSystem().GetUpVector(dir);
	FBXSDK_printf("current UP vector is: %d\n", lUpVector);
	FbxAxisSystem opengl(FbxAxisSystem::EPreDefinedAxisSystem::eOpenGL); 
	opengl.ConvertScene(pFbxScene);
	lUpVector = pFbxScene->GetGlobalSettings().GetAxisSystem().GetUpVector(dir);
	FBXSDK_printf("after scene covert, UP vector is: %d\n", lUpVector);
}

// 深度优先遍历fbx文件中的node节点信息
void FbxCommonTool::NodeExtractWithDepth(FbxScene* pFbxScene, FbxArray<FbxNode *> &destFbxSkeletonArray)
{
	if (pFbxScene)
	{
		FbxNode *pRootNode = pFbxScene->GetRootNode();
		if (pRootNode)
		{
			for (int i = 0; i < pRootNode->GetChildCount(); i++)
			{
				NodeExtractWithDepth(pRootNode->GetChild(i), destFbxSkeletonArray);
			}
		}
	}
}

//递归深度优先搜索
void FbxCommonTool::NodeExtractWithDepth(FbxNode *pFbxNode, FbxArray<FbxNode *> &destFbxSkeletonArray)
{
	if (pFbxNode)
	{
		if (pFbxNode->GetNodeAttribute() == NULL)
		{
			FBXSDK_printf("NULL Node Attribute\n\n");
		}
		else
		{
			FbxNodeAttribute::EType lAttributeType = (pFbxNode->GetNodeAttribute()->GetAttributeType());
			if (lAttributeType == FbxNodeAttribute::eSkeleton)
			{
				destFbxSkeletonArray.Add(pFbxNode);
			}
		}
		for (int i = 0; i < pFbxNode->GetChildCount(); i++)
		{
			NodeExtractWithDepth(pFbxNode->GetChild(i), destFbxSkeletonArray);
		}
	}
}

// 层序优先遍历fbx文件中的node阶段信息
void FbxCommonTool::NodeExtractWithHierarchy(FbxScene* pFbxScene, FbxArray<FbxNode *> &destFbxSkeletonArray)
{
	if (pFbxScene)
	{
		FbxNode *pRootNode = pFbxScene->GetRootNode();
		if (pRootNode)
		{
			for (int i = 0; i < pRootNode->GetChildCount(); i++)
			{
				NodeExtractWithHierarchy(pRootNode->GetChild(i), destFbxSkeletonArray);
			}
		}
	}
}

//递归层次优先搜索
void FbxCommonTool::NodeExtractWithHierarchy(FbxNode *pFbxNode, FbxArray<FbxNode *> &destFbxSkeletonArray)
{
	if (pFbxNode)
	{
		if (pFbxNode->GetNodeAttribute() == NULL)
		{
			FBXSDK_printf("NULL Node Attribute\n\n");
		}
		else
		{
			FbxNodeAttribute::EType lAttributeType = (pFbxNode->GetNodeAttribute()->GetAttributeType());
			if (lAttributeType == FbxNodeAttribute::eSkeleton)
			{
				FbxNode* lChildNode;
				for (int i = 0; i < pFbxNode->GetChildCount(); i++)
				{
					lChildNode = pFbxNode->GetChild(i);
					destFbxSkeletonArray.Add(lChildNode);
				}
				int lNodeChildCount = pFbxNode->GetChildCount();
				while (lNodeChildCount > 0)
				{
					lNodeChildCount--;
					lChildNode = pFbxNode->GetChild(lNodeChildCount);
					NodeExtractWithHierarchy(lChildNode, destFbxSkeletonArray);
				}
			}
		}
	}
}

// 获取节点的全局转换矩阵
FbxAMatrix FbxCommonTool::GetGlobalTransitionMatrix(FbxNode* pFbxNode)
{
	FbxAMatrix lLocalTransitionMatrix;
	FbxAMatrix lGlobalTransitionMatrix;
	FbxAMatrix lParentGlobalTransitionMatrix;

	lLocalTransitionMatrix = GetLocalTransitionMatrix(pFbxNode);

	if (pFbxNode->GetParent())
	{
		lParentGlobalTransitionMatrix = GetGlobalTransitionMatrix(pFbxNode->GetParent());
		lGlobalTransitionMatrix = lParentGlobalTransitionMatrix * lLocalTransitionMatrix;
	}
	else
	{
		lGlobalTransitionMatrix = lLocalTransitionMatrix;
	}

	return lGlobalTransitionMatrix;
}

// 给某节点设置指定的全局转换矩阵
void FbxCommonTool::SetGlobalTransitionMatrix(FbxNode* pFbxNode, const FbxAMatrix &GlobalTransitionMatrix)
{
	FbxAMatrix lLocalTransitionMatrix;
	FbxAMatrix lParentGlobalTransitionMatrix;

	if (pFbxNode->GetParent())
	{
		lParentGlobalTransitionMatrix = GetGlobalTransitionMatrix(pFbxNode->GetParent());
		lLocalTransitionMatrix = lParentGlobalTransitionMatrix.Inverse() * GlobalTransitionMatrix;
	}
	else
	{
		lLocalTransitionMatrix = GlobalTransitionMatrix;
	}

	SetLocalTransitionMatrix(pFbxNode, lLocalTransitionMatrix);
}

// 获取某节点的局部转换矩阵
FbxAMatrix FbxCommonTool::GetLocalTransitionMatrix(FbxNode* pFbxNode)
{
	FbxAMatrix lLocalTransitionMatrix;
	if (pFbxNode)
	{
		lLocalTransitionMatrix.SetT(pFbxNode->LclTranslation.Get());
		lLocalTransitionMatrix.SetR(pFbxNode->LclRotation.Get());
		lLocalTransitionMatrix.SetS(pFbxNode->LclScaling.Get());
	}
	return lLocalTransitionMatrix;
}

// 设置某节点的局部转换矩阵
void FbxCommonTool::SetLocalTransitionMatrix(FbxNode* pFbxNode, const FbxAMatrix &LocalTransitionMatrix)
{
	if (pFbxNode == nullptr)
		return;

	pFbxNode->LclTranslation.Set(LocalTransitionMatrix.GetT());
	pFbxNode->LclRotation.Set(LocalTransitionMatrix.GetR());
	pFbxNode->LclScaling.Set(LocalTransitionMatrix.GetS());
}

// 复制转换矩阵
void FbxCommonTool::CopyTransMatrix(const FbxAMatrix &TransitionMatrix, msg::FbxTransMatrix *pTransMatrixPb)
{
	if (pTransMatrixPb)
	{
		FbxVector4 Position = TransitionMatrix.GetT();
		msg::FbxVector3 *pPosition = pTransMatrixPb->mutable_translation();
		pPosition->set_x(Position[0]);
		pPosition->set_y(Position[1]);
		pPosition->set_z(Position[2]);

		FbxVector4 RotationEuler = TransitionMatrix.GetR();
		FbxVector4 RotationRadian = DegreeToRadian(RotationEuler);
		msg::FbxVector3 *pRotationRadian = pTransMatrixPb->mutable_rotation();
		pRotationRadian->set_x(RotationRadian[0]);
		pRotationRadian->set_y(RotationRadian[1]);
		pRotationRadian->set_z(RotationRadian[2]);

		FbxVector4 Scale = TransitionMatrix.GetS();
		msg::FbxVector3 *pScale = pTransMatrixPb->mutable_scale();
		pScale->set_x(Scale[0]);
		pScale->set_y(Scale[1]);
		pScale->set_z(Scale[2]);

		FbxQuaternion RotationQuat = TransitionMatrix.GetQ();
		msg::FbxVector4 *pRotationQuat = pTransMatrixPb->mutable_quaternion();
		pRotationQuat->set_x(RotationQuat[0]);
		pRotationQuat->set_y(RotationQuat[1]);
		pRotationQuat->set_z(RotationQuat[2]);
		pRotationQuat->set_w(RotationQuat[3]);
	}
}

// 角度转弧度
FbxVector4 FbxCommonTool::DegreeToRadian(const FbxVector4 &DegreeVec)
{
	return DegreeVec * PI / 180.0;
}

// 弧度转角度
FbxVector4 FbxCommonTool::RadianToDegree(const FbxVector4 &RadianVec)
{
	return RadianVec * 180.0 / PI;
}