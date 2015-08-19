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
	m_SurfacePositon = Vector3(0, 1.5f, 1.5f);
	m_pOriSensor = Windows::Devices::Sensors::OrientationSensor::GetDefault();
}

bool DimSketch::Xaml::Controls::Tablet3DViewPanel::AddDrawStroke(IVectorView<InkStroke^>^ strokes)
{
	for each (auto stroke in strokes)
	{
	}
	return true;
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::NotifyPropertyChanged(Platform::String ^ prop)
{

}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::Update(double elapsedTime)
{
	try
	{
		if (m_pOriSensor)
		{
			auto reading = m_pOriSensor->GetCurrentReading();
			auto rq = reading->Quaternion;
			DirectX::Quaternion q(rq->X, rq->Z, -rq->Y, rq->W);
			m_SurfaceOrientation = q;
		}
	}
	catch (Platform::Exception^ exception) // Do nothing if the orientation reading failed
	{
		m_pOriSensor = Windows::Devices::Sensors::OrientationSensor::GetDefault();
	}
	//Panel::Update(elapsedTime);
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::Render()
{
	ClearPanel(DirectX::Colors::Orange);

	//XMMATRIX world = XMMatrixIdentity();
	XMVECTOR campos = m_SurfacePositon;

	XMVECTOR forward = XMVector3Rotate(g_XMNegIdentityR2.v, m_SurfaceOrientation);
	XMVECTOR up = XMVector3Rotate(g_XMIdentityR1.v, m_SurfaceOrientation);
	//campos += forward * CameraDepth;

	XMMATRIX view = XMMatrixLookToRH(campos, forward, up);

	m_pDrawer->SetView(view);
	m_pDrawer->SetProjection(m_Projection);

	DrawAxis();

	Present();
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
	float aspect = _renderTargetWidth / _renderTargetHeight;
	m_Projection = XMMatrixPerspectiveFovRH(XMConvertToRadians(75), aspect, 0.01f, 100.f);

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
