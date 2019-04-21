#pragma once
#include <fbxsdk.h>
#include "FbxAnimationData.pb.h"

#define PI 3.1415926535898
//#define RADIANTODEGREE(x) x *180.0/PI

class FbxCommonTool
{
public:
	static FbxCommonTool* GetInstance();
private:
	FbxCommonTool();
	~FbxCommonTool();
	static FbxCommonTool* fbxCommonTool;
public:

	// 初始化fbx manager
	void InitializeSdkObjects(FbxManager*& pFbxManager, FbxScene*& pFbxScene);

	// 销毁fbx manager
	void DestroySdkObjects(FbxManager* pFbxManager, bool pExitStatus);

	// 保持场景到fbx文件中
	bool SaveScene(FbxManager* pFbxManager, FbxDocument* pFbxScene, const char* pFilename, int pFileFormat = -1, bool pEmbedMedia = false);

	// 导入fbx文件到场景中
	bool LoadScene(FbxManager* pFbxManager, FbxDocument* pFbxScene, const char* pFilename);

	// 将场景转换成3d max场景
	void CovertSceneTo3DMax(FbxScene* pFbxScene);

	// 将场景转换成opengl 场景
	void CovertSceneToOpenGL(FbxScene* pFbxScene);

	// 深度优先遍历fbx文件中的node节点信息
	void NodeExtractWithDepth(FbxScene* pFbxScene, FbxArray<FbxNode *> &destFbxSkeletonArray);

	// 层序优先遍历fbx文件中的node阶段信息
	void NodeExtractWithHierarchy(FbxScene* pFbxScene, FbxArray<FbxNode *> &destFbxSkeletonArray);

	// 获取节点的全局转换矩阵
	FbxAMatrix GetGlobalTransitionMatrix(FbxNode* pFbxNode);
	 
	// 给某节点设置指定的全局转换矩阵
	void SetGlobalTransitionMatrix(FbxNode* pFbxNode, const FbxAMatrix &GlobalTransitionMatrix);

	// 获取某节点的局部转换矩阵
	FbxAMatrix GetLocalTransitionMatrix(FbxNode* pFbxNode);

	// 设置某节点的局部转换矩阵
	void SetLocalTransitionMatrix(FbxNode* pFbxNode, const FbxAMatrix &LocalTransitionMatrix);

	// 复制转换矩阵
	void CopyTransMatrix(const FbxAMatrix &TransitionMatrix, msg::FbxTransMatrix *pTransMatrixPb);

	// 角度转弧度
	FbxVector4 DegreeToRadian(const FbxVector4 &DegreeVec);

	// 弧度转角度
	FbxVector4 RadianToDegree(const FbxVector4 &RadianVec);

protected:
	//递归深度优先搜索
	void NodeExtractWithDepth(FbxNode *pFbxNode, FbxArray<FbxNode *> &destFbxSkeletonArray);

	//递归层次优先搜索
	void NodeExtractWithHierarchy(FbxNode *pFbxNode, FbxArray<FbxNode *> &destFbxSkeletonArray);

};

