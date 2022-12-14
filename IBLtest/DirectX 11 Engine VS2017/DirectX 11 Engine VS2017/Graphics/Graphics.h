//=============================================================================
//
// グラフィックス [Graphics.h]
// Author : GP11B183 28 潘 唯多 (パン　イタ)
//
//=============================================================================
#pragma once
#include "AdapterReader.h"
#include "Shaders.h"
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>
#include "Camera3D.h"
#include "..\\Timer.h"
#include "ImGUI\\imgui.h"
#include "ImGUI\\imgui_impl_win32.h"
#include "ImGUI\\imgui_impl_dx11.h"
#include "RenderableGameObject.h"
#include "Light.h"
#include "Camera2D.h"
#include "Sprite.h"
#include "SkyBox.h"
#include "SpotLight.h"
#include "PointLight.h"
//#include <Effekseer/Effekseer.h>
//#include <Effekseer/src/EffekseerRendererDX11/EffekseerRendererDX11.h>

class Graphics
{
public:
	//初期化
	bool Initialize(HWND hwnd, int width, int height);

	//レンダ
	void RenderFrame();

	//カメラ3D
	Camera3D Camera3D;

	//カメラ2d
	Camera2D camera2D;

	//スプライト2d用
	Sprite sprite;

	//ゲームオブジェクト
	RenderableGameObject gameObject;

	//ゲームオブジェクト
	RenderableGameObject enemy;

	//ステージ
	RenderableGameObject stage;

	//ステージ
	RenderableGameObject test;

	RenderableGameObject quad;

	RenderableGameObject AluminiumInsulator;
	RenderableGameObject Gold;
	RenderableGameObject GunMetal;
	RenderableGameObject Leather;
	RenderableGameObject SuperHeroFabric;
	RenderableGameObject CamoFabric;
	RenderableGameObject GlassVisor;
	RenderableGameObject IronOld;
	RenderableGameObject Rubber;
	RenderableGameObject Wood;
	RenderableGameObject DamagedHelmet;
	
	RenderableGameObject cubesquare[5][5];

	//ステージ
	SkyBox skybox;

	//ライト
	Light light;

	//shader用スプライト
	Sprite disslovenoise;
	float dissolveTheradhold = 0.0f;
	float increaserate = 0.01;
	bool start_dissolve_animation = false;

	//サポートUI描画
	bool showImgui = false;

	//深度ステート有効無効設置
	void SetDepthEnable(bool Enable);

	//prerender
	bool InitializeIBLStatus();

	void InitializeMaterial();

	//game reset
	void ResetGame();
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();
	
	bool InitializeEffekseer();
	void EffekseerUpdate();
	void EffekseerDraw();

	Microsoft::WRL::ComPtr<ID3D11Device> device;//ダイレクトエクスデバイス
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;//ダイレクトエクスデバイスコンテスト
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;//ダイレクトエクススワップ チェーン
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;//ダイレクトエクスレンダターゲットヴュー

	VertexShader vertexshader_2d;//2dバーテックスシェーダー
	VertexShader vertexshader;//バーテックスシェーダー
	VertexShader vertexshader_skyBox;//skyboxバーテックスシェーダー
	PixelShader pixelshader_2d;//2dピクセルシェーダー
	PixelShader pixelshader;//ピクセルシェーダー
	PixelShader pixelmatshader;
	PixelShader pixelshader_skyBox;//skyboxピクセルシェーダー
	PixelShader pixelshader_nolight;//ピクセルシェーダー
	PixelShader ConvolutionPixelShader;
	PixelShader PrefilterMapPixelShader;
	PixelShader IntegrateBRDFPixelShader;

	//対応する定数バッファ
	ConstantBuffer<CB_VS_vertexshader_2d> cb_vs_vertexshader_2d;
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;
	ConstantBuffer<CB_Bone_Info> cb_bone_info;
	ConstantBuffer<CB_PS_light> cb_ps_light;
	ConstantBuffer<CB_PS_IBLSTATUS> cb_ps_iblstatus;
	ConstantBuffer<CB_PS_IBLROUGHNESS> cb_ps_iblroughness;
	

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;//深度ステンシルビュー
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;// 深度ステンシルバッファ
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;// 深度ステンシルステート
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthUnenableStencilState;// 深度ステンシルステート

	//ラスタライザステート
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_CullFront;

	//ブレンドステート
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> spriteFont;

	//サンプルステート
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pinkTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> grassTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pavementTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> brdfLUT;

	//タイマー
	int windowWidth = 0;
	int windowHeight = 0;
	Timer fpsTimer;

	//ライト
	std::vector<PointLight*> pointLights;
	std::vector<SpotLight*> spotLights;

	//IBL
	ID3D11Texture2D* skyIBLtex;
	//ID3D11RenderTargetView* skyIBLRTV[6];
	ID3D11ShaderResourceView* skyIBLSRV;

	ID3D11Texture2D* envMaptex;
	//ID3D11RenderTargetView* envMapRTV[6];
	ID3D11ShaderResourceView* envMapSRV;

	ID3D11Texture2D* brdfLUTtex;
	//ID3D11RenderTargetView* brdfLUTRTV;
	ID3D11ShaderResourceView* brdfLUTSRV;

	//material
	//Albedo Shader Resource Views(SRVs)
	ID3D11ShaderResourceView* AluminiumInsulator_Albedo;
	ID3D11ShaderResourceView* Gold_Albedo;
	ID3D11ShaderResourceView* GunMetal_Albedo;
	ID3D11ShaderResourceView* Leather_Albedo;
	ID3D11ShaderResourceView* SuperHeroFabric_Albedo;
	ID3D11ShaderResourceView* CamoFabric_Albedo;
	ID3D11ShaderResourceView* GlassVisor_Albedo;
	ID3D11ShaderResourceView* IronOld_Albedo;
	ID3D11ShaderResourceView* Rubber_Albedo;
	ID3D11ShaderResourceView* Wood_Albedo;
	ID3D11ShaderResourceView* DamagedHelmet_Albedo;

	//Normal Shader Resource Views(SRVs)
	ID3D11ShaderResourceView* AluminiumInsulator_Normal;
	ID3D11ShaderResourceView* Gold_Normal;
	ID3D11ShaderResourceView* GunMetal_Normal;
	ID3D11ShaderResourceView* Leather_Normal;
	ID3D11ShaderResourceView* SuperHeroFabric_Normal;
	ID3D11ShaderResourceView* CamoFabric_Normal;
	ID3D11ShaderResourceView* GlassVisor_Normal;
	ID3D11ShaderResourceView* IronOld_Normal;
	ID3D11ShaderResourceView* Rubber_Normal;
	ID3D11ShaderResourceView* Wood_Normal;
	ID3D11ShaderResourceView* DamagedHelmet_Normal;

	//Metallic Shader Resource Views(SRVs)
	ID3D11ShaderResourceView* AluminiumInsulator_Metallic;
	ID3D11ShaderResourceView* Gold_Metallic;
	ID3D11ShaderResourceView* GunMetal_Metallic;
	ID3D11ShaderResourceView* Leather_Metallic;
	ID3D11ShaderResourceView* SuperHeroFabric_Metallic;
	ID3D11ShaderResourceView* CamoFabric_Metallic;
	ID3D11ShaderResourceView* GlassVisor_Metallic;
	ID3D11ShaderResourceView* IronOld_Metallic;
	ID3D11ShaderResourceView* Rubber_Metallic;
	ID3D11ShaderResourceView* Wood_Metallic;
	ID3D11ShaderResourceView* DamagedHelmet_Metallic;

	//Roughness Shader Resource Views(SRVs)
	ID3D11ShaderResourceView* AluminiumInsulator_Rough;
	ID3D11ShaderResourceView* Gold_Rough;
	ID3D11ShaderResourceView* GunMetal_Rough;
	ID3D11ShaderResourceView* Leather_Rough;
	ID3D11ShaderResourceView* SuperHeroFabric_Rough;
	ID3D11ShaderResourceView* CamoFabric_Rough;
	ID3D11ShaderResourceView* GlassVisor_Rough;
	ID3D11ShaderResourceView* IronOld_Rough;
	ID3D11ShaderResourceView* Rubber_Rough;
	ID3D11ShaderResourceView* Wood_Rough;
	ID3D11ShaderResourceView* DamagedHelmet_Rough;



	//effekseer
	//Effekseer::ManagerRef manager;
	//Effekseer::EffectRef effect;
	//EffekseerRendererDX11::RendererRef renderer;
	//int32_t time = 0;
	//Effekseer::Handle handle = 0;
};