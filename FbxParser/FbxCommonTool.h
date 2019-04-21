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

	// ��ʼ��fbx manager
	void InitializeSdkObjects(FbxManager*& pFbxManager, FbxScene*& pFbxScene);

	// ����fbx manager
	void DestroySdkObjects(FbxManager* pFbxManager, bool pExitStatus);

	// ���ֳ�����fbx�ļ���
	bool SaveScene(FbxManager* pFbxManager, FbxDocument* pFbxScene, const char* pFilename, int pFileFormat = -1, bool pEmbedMedia = false);

	// ����fbx�ļ���������
	bool LoadScene(FbxManager* pFbxManager, FbxDocument* pFbxScene, const char* pFilename);

	// ������ת����3d max����
	void CovertSceneTo3DMax(FbxScene* pFbxScene);

	// ������ת����opengl ����
	void CovertSceneToOpenGL(FbxScene* pFbxScene);

	// ������ȱ���fbx�ļ��е�node�ڵ���Ϣ
	void NodeExtractWithDepth(FbxScene* pFbxScene, FbxArray<FbxNode *> &destFbxSkeletonArray);

	// �������ȱ���fbx�ļ��е�node�׶���Ϣ
	void NodeExtractWithHierarchy(FbxScene* pFbxScene, FbxArray<FbxNode *> &destFbxSkeletonArray);

	// ��ȡ�ڵ��ȫ��ת������
	FbxAMatrix GetGlobalTransitionMatrix(FbxNode* pFbxNode);
	 
	// ��ĳ�ڵ�����ָ����ȫ��ת������
	void SetGlobalTransitionMatrix(FbxNode* pFbxNode, const FbxAMatrix &GlobalTransitionMatrix);

	// ��ȡĳ�ڵ�ľֲ�ת������
	FbxAMatrix GetLocalTransitionMatrix(FbxNode* pFbxNode);

	// ����ĳ�ڵ�ľֲ�ת������
	void SetLocalTransitionMatrix(FbxNode* pFbxNode, const FbxAMatrix &LocalTransitionMatrix);

	// ����ת������
	void CopyTransMatrix(const FbxAMatrix &TransitionMatrix, msg::FbxTransMatrix *pTransMatrixPb);

	// �Ƕ�ת����
	FbxVector4 DegreeToRadian(const FbxVector4 &DegreeVec);

	// ����ת�Ƕ�
	FbxVector4 RadianToDegree(const FbxVector4 &RadianVec);

protected:
	//�ݹ������������
	void NodeExtractWithDepth(FbxNode *pFbxNode, FbxArray<FbxNode *> &destFbxSkeletonArray);

	//�ݹ�����������
	void NodeExtractWithHierarchy(FbxNode *pFbxNode, FbxArray<FbxNode *> &destFbxSkeletonArray);

};

