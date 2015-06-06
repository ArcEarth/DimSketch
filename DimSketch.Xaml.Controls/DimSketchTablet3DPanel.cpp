#include "pch.h"
#include "DimSketchTablet3DPanel.h"
#include "DirectXHelper.h"
#include <DirectXColors.h>

using namespace DimSketch::Xaml::Controls::Base;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;

DimSketch::Xaml::Controls::Tablet3DViewPanel::Tablet3DViewPanel()
	: Panel()
{
	m_CameraDepth = 5.0f;
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::NotifyPropertyChanged(Platform::String ^ prop)
{

}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::Update(double elapsedTime)
{
	//Panel::Update(elapsedTime);
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::Render()
{
	BeginRender(nullptr, nullptr, DirectX::Colors::White.f);

	// bind viewport the size of the control
	//const D3D11_VIEWPORT backBufferView = CD3D11_VIEWPORT(0.0f, 0.0f, _renderTargetWidth, _renderTargetHeight);
	//_d3dContext->RSSetViewports(1, &backBufferView);

	XMMATRIX world = XMMatrixIdentity();
	XMVECTOR campos = XMVectorSet(0, m_CameraDepth, m_CameraDepth, 0);
	XMMATRIX view = XMMatrixLookAtRH(campos, g_XMZero.v, g_XMIdentityR1.v);
	float aspect = _renderTargetWidth / _renderTargetHeight;
	XMMATRIX proj = XMMatrixPerspectiveFovRH(XMConvertToRadians(75), aspect, 0.01f, 100.f);

	m_pDrawer->SetView(view);
	m_pDrawer->SetProjection(proj);

	DrawAxis();

	EndRender();
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::ResetDeviceResources()
{
	Panel::ResetDeviceResources();
	m_pDrawer.reset();
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::CreateDeviceResources()
{
	Panel::CreateDeviceResources();
	m_pDrawer = std::make_shared<PrimitveDrawer>(_d3dContext.Get());
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::CreateSizeDependentResources()
{
	Panel::CreateSizeDependentResources();
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::StartRenderLoop()
{
	Panel::StartRenderLoop();
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::StopRenderLoop()
{
	Panel::StopRenderLoop();
}

DimSketch::Xaml::Controls::Tablet3DViewPanel::~Tablet3DViewPanel()
{
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::DrawAxis()
{
	float ub = 5, lb = -5, majorIdent = 1, minorIdent = 0.25f;

	m_pDrawer->DrawSphere({ .0f,.0f,.0f,.1f }, Colors::Cyan);

	float Ar = 0.03f, Al = ub, Almr = Al - Ar, Alpr = Al + Ar;
	m_pDrawer->Begin();

	for (float x = lb; x <= ub; x += minorIdent)
	{
		m_pDrawer->DrawLine({ -Al,.0f,x }, { Al,.0f,x }, Colors::LightGray);
		m_pDrawer->DrawLine({ x,.0f,-Al }, { x,.0f,Al }, Colors::LightGray);
	}

	for (float x = lb; x <= ub; x += majorIdent)
	{
		m_pDrawer->DrawLine({ -Al,.0f,x }, { Al,.0f,x }, Colors::Black);
		m_pDrawer->DrawLine({ x,.0f,-Al }, { x,.0f,Al }, Colors::Black);
	}

	m_pDrawer->DrawLine({ -Al,.0f,.0f }, { Al,.0f,.0f }, Colors::Red);
	m_pDrawer->DrawLine({ .0f,-Al,.0f }, { .0f,Al,.0f }, Colors::Green);
	m_pDrawer->DrawLine({ .0f,.0f,-Al }, { .0f,.0f,Al }, Colors::Blue);


	m_pDrawer->DrawTriangle({ Alpr,.0f,.0f }, { Almr,Ar,.0f }, { Almr,-Ar,.0f }, Colors::Red);
	m_pDrawer->DrawTriangle({ Alpr,.0f,.0f }, { Almr,-Ar,.0f }, { Almr,Ar,.0f }, Colors::Red);
	m_pDrawer->DrawTriangle({ Alpr,.0f,.0f }, { Almr,.0f,Ar }, { Almr,.0f,-Ar }, Colors::Red);
	m_pDrawer->DrawTriangle({ Alpr,.0f,.0f }, { Almr,.0f,-Ar }, { Almr,.0f,Ar }, Colors::Red);

	m_pDrawer->DrawTriangle({ .0f,Alpr,.0f }, { -Ar,Almr,.0f }, { Ar,Almr,.0f }, Colors::Green);
	m_pDrawer->DrawTriangle({ .0f,Alpr,.0f }, { Ar,Almr,.0f }, { -Ar,Almr,.0f }, Colors::Green);
	m_pDrawer->DrawTriangle({ .0f,Alpr,.0f }, { .0f,Almr,-Ar }, { .0f,Almr,Ar }, Colors::Green);
	m_pDrawer->DrawTriangle({ .0f,Alpr,.0f }, { .0f,Almr,Ar }, { .0f,Almr,-Ar }, Colors::Green);

	m_pDrawer->DrawTriangle({ .0f,.0f,Alpr }, { Ar,.0f,Almr }, { -Ar,.0f,Almr }, Colors::Blue);
	m_pDrawer->DrawTriangle({ .0f,.0f,Alpr }, { -Ar,.0f,Almr }, { Ar,.0f,Almr }, Colors::Blue);
	m_pDrawer->DrawTriangle({ .0f,.0f,Alpr }, { .0f,Ar,Almr }, { .0f,-Ar,Almr }, Colors::Blue);
	m_pDrawer->DrawTriangle({ .0f,.0f,Alpr }, { .0f,-Ar,Almr }, { .0f,Ar,Almr }, Colors::Blue);
	m_pDrawer->End();
}
