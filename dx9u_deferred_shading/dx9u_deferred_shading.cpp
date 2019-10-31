//-------------------------------------------------------------------------------
//           Name: dx9u_deferred_shading.cpp
//         Author: Nikita Kindt (nk47@bk.ru)
//  Last Modified: 17/06/06
//    Description: This sample demonstrates how to use Deferred Shading
//				   with Direct3D 9. The idea behind Deferred Shading is not
//				   really new and was first introduced by Michael Deering 
//                 at SIGGRAPH 1988.
//				   The theory is simple: render all data of the geometry
//				   like position, normals, diffuse map, environment map etc.
//				   to multiple render targets and apply lighting later
//				   as a postprocess. While lighting is computed the multiple
//                 render targets are used to read out the data stored
//				   previously and render the final colors to a plane which
//				   covers the entire screen.
//				   The Deferred Shading technique brings few optimizations
//				   but has also some disadvantages. While Deferred Shading
//				   needs a high-end GPU (floating point textures / multiple
//				   render targets) which gurantees the incompatibility to older
//				   hardware it can be used to perfectly batch rendering.
//				   Details about various advatages / disadvantages are mentioned
//				   later in the code.
//-------------------------------------------------------------------------------

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <crtdbg.h>
#include <assert.h>

#ifndef _DEBUG
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
#else
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9d.lib" )
#endif

// application
int	g_iWindowWidth = 800;
int	g_iWindowHeight = 600;
bool g_bFullScreen = false;
HINSTANCE g_hAppInstance = NULL;
HWND g_hAppWindow = NULL;

// light
D3DXVECTOR4 g_vLightDiffuse(1.0f, 1.0f, 1.0f, 0.0f);
D3DXVECTOR4 g_vLightSpecular(1.0f, 1.0f, 1.0f, 1.0f);
D3DXMATRIX g_matTransformLight;

// direct3d9
IDirect3D9* g_pd3d = NULL;
IDirect3DDevice9* g_pd3dDevice = NULL;
ID3DXEffect* g_pd3dEffect = NULL;
IDirect3DTexture9* g_pd3dRTSceneMaterialMap = NULL;
IDirect3DTexture9* g_pd3dRTSceneNormalMap = NULL;
IDirect3DTexture9* g_pd3dRTScenePositionXYMap = NULL;
IDirect3DTexture9* g_pd3dRTScenePositionZMap = NULL;

// mesh
D3DXMESHCONTAINER g_d3dMeshContainer;
IDirect3DTexture9* g_pd3dMeshTexture = NULL;
D3DXMATRIX g_matTransformMesh;

// matrices
D3DXMATRIX g_matView;
D3DXMATRIX g_matViewInverse;
D3DXMATRIX g_matProjection;
D3DXMATRIX g_matViewProjection;
D3DXMATRIX g_matIdentity(1, 0, 0, 0,
						 0, 1, 0, 0,
						 0, 0, 1, 0,
						 0, 0, 0, 1);

float g_fTimeFact = 1.0f;
bool g_bMousing = false;
float g_fMousingX = 0.0f;



float GetTime()
{
	static __int64 s_i64CounterLast = 0;
	static double s_dTime = 0;

	if(s_i64CounterLast == 0)
		QueryPerformanceCounter((LARGE_INTEGER*)&s_i64CounterLast);

	__int64 i64Counter;
	QueryPerformanceCounter((LARGE_INTEGER*)&i64Counter);

	__int64 i64CounterFrq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&i64CounterFrq);

	s_dTime += ((double)(i64Counter - s_i64CounterLast)) / ((double)i64CounterFrq) * g_fTimeFact;

	s_i64CounterLast = i64Counter;

	return (float)s_dTime;
}

D3DXMESHCONTAINER* CreateMeshFromX(const char* pcFilename,D3DXMESHCONTAINER* pxMeshContainerOut)
{
	D3DXMATERIAL* pxMaterials = NULL;
	ID3DXBuffer* pd3dAdjacencyBuffer = NULL;
	ID3DXBuffer* pd3dMaterialBuffer = NULL;
	ID3DXBuffer* pd3dEffectInstanceBuffer = NULL;

	HRESULT hr = D3DXLoadMeshFromX(pcFilename,
					D3DXMESH_MANAGED,
					g_pd3dDevice,
					&pd3dAdjacencyBuffer,
					&pd3dMaterialBuffer,
					&pd3dEffectInstanceBuffer,
					&pxMeshContainerOut->NumMaterials,
					&pxMeshContainerOut->MeshData.pMesh);
	if(FAILED(hr))
	{
		MessageBox(NULL,"couldn't load mesh from file!",
					"error",MB_ICONERROR | MB_OK);
		return NULL;
	}

	pxMeshContainerOut->MeshData.Type = D3DXMESHTYPE_MESH;

	pxMaterials = (D3DXMATERIAL*)pd3dMaterialBuffer->GetBufferPointer();

	if(pxMeshContainerOut->NumMaterials > 0)
	{
		pxMeshContainerOut->pMaterials = new D3DXMATERIAL[pxMeshContainerOut->NumMaterials];

		for(int iMaterial = 0; iMaterial < (int)pxMeshContainerOut->NumMaterials ; iMaterial++)
		{
			pxMeshContainerOut->pMaterials[iMaterial].MatD3D = pxMaterials[iMaterial].MatD3D;
			pxMeshContainerOut->pMaterials[iMaterial].pTextureFilename = NULL;
		}
	}

	hr = D3DXCreateTextureFromFile(g_pd3dDevice,"tiles.dds",&g_pd3dMeshTexture);
	if(FAILED(hr))
	{
		MessageBox(NULL,"couldn't load \"tiles.dds\" from file!",
					"error",MB_ICONERROR | MB_OK);
		return NULL;
	}

	pd3dAdjacencyBuffer->Release();
	pd3dMaterialBuffer->Release();
	pd3dEffectInstanceBuffer->Release();

	return pxMeshContainerOut;
}

void DestroyMesh()
{
	if(g_d3dMeshContainer.pMaterials)
		delete [] g_d3dMeshContainer.pMaterials;

	if(g_d3dMeshContainer.MeshData.pMesh)
		g_d3dMeshContainer.MeshData.pMesh->Release();

	if(g_pd3dMeshTexture)
		g_pd3dMeshTexture->Release();
}

void RenderMesh()
{
	g_pd3dEffect->SetTechnique("buildPass");
	g_pd3dEffect->SetMatrix("c_mWorld",&g_matTransformMesh);

	unsigned int iNumPasses;
	g_pd3dEffect->Begin(&iNumPasses,D3DXFX_DONOTSAVESTATE);

	for(int iMaterial = 0; iMaterial < (int)g_d3dMeshContainer.NumMaterials; iMaterial++)
	{
		// set texture
		g_pd3dEffect->SetTexture("c_tDiffuseMap",g_pd3dMeshTexture);

		// set material properties
		g_pd3dEffect->SetVector("c_vMaterialAmbient",(D3DXVECTOR4*)&(g_d3dMeshContainer.pMaterials[iMaterial].MatD3D.Ambient));
		g_pd3dEffect->SetVector("c_vMaterialDiffuse",(D3DXVECTOR4*)&(g_d3dMeshContainer.pMaterials[iMaterial].MatD3D.Diffuse));
		g_pd3dEffect->SetFloat("c_fSpecularPower",g_d3dMeshContainer.pMaterials[iMaterial].MatD3D.Power);

		// render mesh (by subsets)
		for(int iPass = 0; iPass < (int)iNumPasses; iPass++)
		{
			g_pd3dEffect->BeginPass(iPass);
			g_d3dMeshContainer.MeshData.pMesh->DrawSubset(iMaterial);
			g_pd3dEffect->EndPass();
		}
	}

	g_pd3dEffect->End();
}

// create effect from specified file and output errors if any
ID3DXEffect* CreateEffect(const char* pcFilename)
{
	ID3DXBuffer* pd3dErrors = NULL;
	ID3DXEffect* pd3dEffect = NULL;

	D3DXCreateEffectFromFile(g_pd3dDevice, pcFilename,NULL, NULL,
		D3DXFX_NOT_CLONEABLE | D3DXSHADER_SKIPOPTIMIZATION | D3DXSHADER_DEBUG,
		NULL, &pd3dEffect, &pd3dErrors);

	if(pd3dErrors)
	{
		MessageBox(NULL, (const char*)pd3dErrors->GetBufferPointer(), pcFilename, 0);
		pd3dErrors->Release();
		exit(-1);
	}
	if(!pd3dEffect)
	{
		MessageBox(NULL, "couldn't load shader", pcFilename, 0);
		exit(-1);
	}

	HRESULT hr = pd3dEffect->ValidateTechnique(pd3dEffect->GetTechnique(0));

	return pd3dEffect;
}

// create 4 render targets
// note: current graphic cards do not support multiple render targets
// with different bit depth. in our case all are 32bit!
// note: one disadvantage of deferred shading is the large usage of
// video ram. here we use 800x600x4x4 bytes of memory ~ 7.3 MB (1600x1200x4x4 ~ 29 MB)
// memory usage is growing as new geometry data is added.
void CreateRenderTargets()
{
	// Material-Map: A8R8G8B8
	g_pd3dDevice->CreateTexture(g_iWindowWidth,g_iWindowHeight,1,D3DUSAGE_RENDERTARGET, 
				D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,&g_pd3dRTSceneMaterialMap,NULL);

	// Normal-Map: X8R8G8B8
	g_pd3dDevice->CreateTexture(g_iWindowWidth,g_iWindowHeight,1,D3DUSAGE_RENDERTARGET, 
				D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT,&g_pd3dRTSceneNormalMap,NULL);

	// PositionXY-Map: R16G16F
	g_pd3dDevice->CreateTexture(g_iWindowWidth,g_iWindowHeight,1,D3DUSAGE_RENDERTARGET, 
				D3DFMT_G16R16F,D3DPOOL_DEFAULT,&g_pd3dRTScenePositionXYMap,NULL);

	// PositionXY-Map: R16G16F
	g_pd3dDevice->CreateTexture(g_iWindowWidth,g_iWindowHeight,1,D3DUSAGE_RENDERTARGET, 
				D3DFMT_G16R16F,D3DPOOL_DEFAULT,&g_pd3dRTScenePositionZMap,NULL);
}

// handles window messages
LRESULT WINAPI MessageHandler(HWND hWindow,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static POINT pntPrevious = { 0, 0 };
	static POINT pntCurrent = { 0, 0 };

    switch(msg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
            switch (wParam)
            {
            case VK_PRIOR:  g_fTimeFact *= 1.05f; break;
            case VK_NEXT:   g_fTimeFact /= 1.05f; break;
            case VK_ESCAPE: PostQuitMessage(0);   break;
            };
            return 0;
		case WM_LBUTTONDOWN:
			pntPrevious.y = pntCurrent.x = (LONG)(HIWORD(lParam));
			g_bMousing = true;
			break;
		case WM_LBUTTONUP:
			g_bMousing = false;
			break;
		case WM_MOUSEMOVE:
			if(g_bMousing)
			{
				pntCurrent.y = (LONG)(HIWORD(lParam));				
				if(pntCurrent.y != pntPrevious.y)
				{
					g_fMousingX -= (float)(pntCurrent.y - pntPrevious.y) * 0.005;
					pntPrevious.y = pntCurrent.y;
				}
			}
			break;
        default: break;
    }

	return DefWindowProc(hWindow, msg, wParam, lParam);
}

// initialize application window, direct3d, render targets and mesh
void InitApp()
{
	if(MessageBox(0,"- Use mouse to rotate object\n"
					"- Press ESC to exit\n\n"
					"Run in fullscreen mode?","Deferred Shading Sample",MB_YESNO) == IDYES)
		g_bFullScreen = true;
	else g_bFullScreen = false;

	// create window
    WNDCLASS xWindowClass = { CS_CLASSDC,MessageHandler,0L,0L,
				 g_hAppInstance,NULL,NULL,NULL,NULL,"d3dapp" };
    RegisterClass(&xWindowClass);

	RECT rctWindowRect = { 0, 0, g_iWindowWidth, g_iWindowHeight };
    AdjustWindowRectEx(&rctWindowRect,WS_OVERLAPPEDWINDOW,FALSE,WS_EX_APPWINDOW);

	g_hAppWindow = CreateWindow("d3dapp","Deferred Shading Sample"
		" - Use mouse to rotate teapot - ESC to exit",
        WS_OVERLAPPEDWINDOW,50,50, 
		rctWindowRect.right - rctWindowRect.left, 
		rctWindowRect.bottom - rctWindowRect.top,
        GetDesktopWindow(),NULL,g_hAppInstance,NULL);

    ShowWindow(g_hAppWindow,SW_SHOWDEFAULT);

	// initialize direct3d
	g_pd3d = Direct3DCreate9(D3D_SDK_VERSION);
	if(g_pd3d == NULL)
	{
		MessageBox(g_hAppWindow, 
			"This program requires DirectX 9.0c",
			"critical error",MB_ICONERROR | MB_OK);
		exit(-1);
	}

	D3DPRESENT_PARAMETERS xPresentParameters;    
	ZeroMemory(&xPresentParameters, sizeof(xPresentParameters));
	xPresentParameters.BackBufferWidth = g_iWindowWidth;
	xPresentParameters.BackBufferHeight = g_iWindowHeight;
	xPresentParameters.Windowed = !g_bFullScreen;
	xPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	xPresentParameters.EnableAutoDepthStencil = true;
	xPresentParameters.AutoDepthStencilFormat = D3DFMT_D24S8;
	xPresentParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
	xPresentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	xPresentParameters.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

	HRESULT hr = g_pd3d->CreateDevice(
		D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,g_hAppWindow, 
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE, 
		&xPresentParameters,&g_pd3dDevice);

	if(FAILED(hr))
	{
		MessageBox(g_hAppWindow, 
			"Direct3D-CreateDevice failed !\n"
			"hardware acceleration required", 
			"critical error",MB_ICONERROR | MB_OK);
		exit(-1);
	}

	// create data
	g_pd3dEffect = CreateEffect("deferred.fx");

	CreateMeshFromX("teapot.x",&g_d3dMeshContainer);

	CreateRenderTargets();

	// create matrices mandatory for shader
	D3DXMatrixPerspectiveFovLH(&g_matProjection,
		45.0f / 180.0f * 3.14159f,
		(float)g_iWindowWidth / (float)g_iWindowHeight,
		0.5f, 100.0f);

	D3DXMatrixLookAtLH(&g_matView, 
		&D3DXVECTOR3(0, 0,-14),
		&D3DXVECTOR3(0, 0, 0 ),
		&D3DXVECTOR3(0, 1, 0));

	D3DXMatrixInverse(&g_matViewInverse,NULL,&g_matView);

	D3DXMatrixMultiply(&g_matViewProjection,&g_matView,&g_matProjection);	
}

void DestroyRenderTargets()
{
	if(g_pd3dRTSceneMaterialMap)
		g_pd3dRTSceneMaterialMap->Release();
	if(g_pd3dRTSceneNormalMap)
		g_pd3dRTSceneNormalMap->Release();
	if(g_pd3dRTScenePositionXYMap)
		g_pd3dRTScenePositionXYMap->Release();
	if(g_pd3dRTScenePositionZMap)
		g_pd3dRTScenePositionZMap->Release();
}

// compute and pass matrices to effect
void SetupMatrices()
{
	// rotate object from mouse input around the x-axis
	D3DXMATRIX matXAxisRotation;
	D3DXMatrixIdentity(&g_matTransformMesh);
	D3DXMatrixRotationX(&matXAxisRotation,g_fMousingX);
	D3DXMatrixMultiply(&g_matTransformMesh,&g_matTransformMesh,&matXAxisRotation);

	// rotate light around geometry
	D3DXMATRIX matRotation, matTranslation;
	D3DXMatrixRotationY(&matRotation,GetTime());
	D3DXMatrixTranslation(&matTranslation,10,0,0);
	D3DXMatrixMultiply(&g_matTransformLight,&matTranslation,&matRotation);

	g_pd3dEffect->SetMatrix("c_mView",&g_matView);
	g_pd3dEffect->SetMatrix("c_mViewInverse",&g_matViewInverse);
	g_pd3dEffect->SetMatrix("c_mProjection",&g_matProjection);
	g_pd3dEffect->SetMatrix("c_mViewProjection",&g_matViewProjection);
}

// clear given texture with given color
void ClearTexture(IDirect3DTexture9* pd3dTexture,D3DCOLOR xColor)
{
	IDirect3DSurface9* pd3dSurface;
	pd3dTexture->GetSurfaceLevel(0,&pd3dSurface);
	g_pd3dDevice->ColorFill(pd3dSurface,NULL,xColor);
	pd3dSurface->Release();
}

// set render target on the given level (iRenderTargetIdx: 0 - 3)
void SetRenderTarget(int iRenderTargetIdx,IDirect3DTexture9* pd3dRenderTargetTexture)
{
	IDirect3DSurface9* pd3dSurface;
	pd3dRenderTargetTexture->GetSurfaceLevel(0,&pd3dSurface);
	g_pd3dDevice->SetRenderTarget(iRenderTargetIdx,pd3dSurface);
	pd3dSurface->Release();
}

// 1st pass - render geometry to multiple render targets
// note: we render the scene only once and apply lighting (multiple lights) later
void RenderBuildPass()
{
	// clear render targets from previous content
	ClearTexture(g_pd3dRTSceneMaterialMap,0x00000000);
	ClearTexture(g_pd3dRTSceneNormalMap,0x00000000);
	ClearTexture(g_pd3dRTScenePositionXYMap,0x00000000);
	ClearTexture(g_pd3dRTScenePositionZMap,0x00000000);

	// set each render target by index
	// note: you can check the D3DCAPS9::NumSimultaneousRTs to obtain
	// the number of max. simultaneous render targets supported by device
	SetRenderTarget(0,g_pd3dRTSceneMaterialMap);
	SetRenderTarget(1,g_pd3dRTSceneNormalMap);
	SetRenderTarget(2,g_pd3dRTScenePositionXYMap);
	SetRenderTarget(3,g_pd3dRTScenePositionZMap);

	RenderMesh();

	// disable render targets
	IDirect3DSurface9* pd3dBackBufferSurface;
	g_pd3dDevice->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pd3dBackBufferSurface);
	g_pd3dDevice->SetRenderTarget(0,pd3dBackBufferSurface);
	pd3dBackBufferSurface->Release();
	g_pd3dDevice->SetRenderTarget(1,NULL);
	g_pd3dDevice->SetRenderTarget(2,NULL);
	g_pd3dDevice->SetRenderTarget(3,NULL);
}

// 2nd pass - apply light as 2D post process
// note: the whole scene is affected by this pass which
// can be used as well to produce effects like fog, blur, etc.
void RenderLightPass()
{
	static float fWidth = (float)g_iWindowWidth;
	static float fHeight = (float)g_iWindowHeight;

	struct PlaneVertex {
		float x, y, z, w;
		float u, v;
	};
	// create plane once (screen size) on which the scene is rendered
	static PlaneVertex axPlaneVertices[] =
	{
		{ 0,		 0,		  .5f, 1, 0 + .5f / fWidth,	0 + .5f / fHeight },
		{ fWidth, 0,		  .5f, 1, 1 + .5f / fWidth,	0 + .5f / fHeight },
		{ fWidth, fHeight,    .5f, 1, 1 + .5f / fWidth,	1 + .5f / fHeight },
		{ 0,		 fHeight, .5f, 1, 0 + .5f / fWidth,	1 + .5f / fHeight }
	};

	g_pd3dEffect->SetTechnique("lightPass");

	// pass light data to effect
	D3DXVECTOR4 vLightPos(g_matTransformLight._41,
						  g_matTransformLight._42,
						  g_matTransformLight._43,
						  g_matTransformLight._44);
	D3DXVECTOR4 vObjectPos(g_matTransformMesh._41,
						   g_matTransformMesh._42,
						   g_matTransformMesh._43,
						   g_matTransformMesh._44);
	D3DXVECTOR4 vLightDir = vLightPos - vObjectPos;
	D3DXVec4Normalize(&vLightDir,&vLightDir);
	g_pd3dEffect->SetVector("c_vLightDir",&vLightDir);
	g_pd3dEffect->SetVector("c_vLightPos",&vLightPos);
	g_pd3dEffect->SetVector("c_vLightDiffuse",&g_vLightDiffuse);
	g_pd3dEffect->SetVector("c_vLightSpecular",&g_vLightSpecular);
	g_pd3dEffect->SetFloat("c_fSpecularPower",8.0f);

	// set render targets as 2d textures.
	// the light pass will make use of them to correctly produce
	// the final image (see lightPass in deferred.fx)
	g_pd3dEffect->SetTexture("c_tSceneMaterialMap",g_pd3dRTSceneMaterialMap);
	g_pd3dEffect->SetTexture("c_tSceneNormalMap",g_pd3dRTSceneNormalMap);
	g_pd3dEffect->SetTexture("c_tScenePositionXYMap",g_pd3dRTScenePositionXYMap);
	g_pd3dEffect->SetTexture("c_tScenePositionZMap",g_pd3dRTScenePositionZMap);

	// render the plane and construct final image in pixel shader (psLighting)
	int iNumPasses;
	g_pd3dEffect->Begin((UINT*)&iNumPasses,0);
	for(int iPassIdx = 0; iPassIdx < iNumPasses; iPassIdx++)
	{
		g_pd3dEffect->BeginPass(iPassIdx);
		g_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
		g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, axPlaneVertices, sizeof(PlaneVertex));
		g_pd3dEffect->EndPass();
	}
	g_pd3dEffect->End();
}

void RenderScene()
{
    if(SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
		g_pd3dDevice->Clear(0, NULL,
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
			0x00000000, 1.0f, 0);
	    
        SetupMatrices();
		RenderBuildPass();
		RenderLightPass();

		g_pd3dDevice->EndScene();
    }
    g_pd3dDevice->Present(NULL, NULL, NULL, NULL);    
}

void MessageLoop()
{
    MSG msg = {0, 0, 0, 0, 0, 0};
    
    while(msg.message != WM_QUIT)
    {
        if(PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) != 0)
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            RenderScene();
        }
    }
}

// entry point:
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, INT)
{
    g_hAppInstance = hInstance;

	// init app, direct3d, scene
    InitApp();

	// enter message loop
    MessageLoop();

	// end of loop - cleanup everything
	DestroyRenderTargets();
	DestroyMesh();

	if(g_pd3dEffect)
		g_pd3dEffect->Release();  
	if(g_pd3dDevice)
		g_pd3dDevice->Release();
	if(g_pd3d)
		g_pd3d->Release();

	UnregisterClass("d3dapp", g_hAppInstance);
}