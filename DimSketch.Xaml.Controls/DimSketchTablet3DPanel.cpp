#include "pch.h"
#include "DimSketchTablet3DPanel.h"
#include "DirectXHelper.h"
#include <DirectXColors.h>
#include <iostream>

using namespace DimSketch::Xaml::Controls::Base;

using namespace Concurrency;
using namespace DirectX;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Data;

DimSketch::Xaml::Controls::Tablet3DViewPanel::Tablet3DViewPanel()
	: Panel()
{
	m_SurfacePositon = Vector3(.0f, 1.5f, 1.5f);
	m_pOriSensor = Windows::Devices::Sensors::OrientationSensor::GetDefault();
}

bool DimSketch::Xaml::Controls::Tablet3DViewPanel::AddDrawStroke(IVectorView<InkStroke^>^ strokes)
{
	XMMATRIX s2w = m_ScreenToWorld;
	for each (auto stroke in strokes)
	{
		auto strokps = stroke->GetInkPoints();
		std::vector<Vector3> points(strokps->Size);
		
		for (size_t i = 0; i < strokps->Size; i++)
		{
			auto p2 = strokps->GetAt(i);

			Vector2 xy (p2->Position.X,p2->Position.Y);
			xy = xy / m_ViewPortSize;
			xy = xy * Vector2(2.0f, -2.0f) + Vector2(-1.0f, 1.0f);
			//xy *= m_ViewPortSize;

			//Screen space position
			XMVECTOR posSs = XMVectorSet(
				xy.x,
				xy.y,
				m_SliceDepthScreenSpace,
				1.0f);

			posSs = XMVector3TransformCoord(posSs, s2w);
			points[i] = posSs;
		}

		critical_section::scoped_lock lock(_criticalSection);

		m_Strokes.push_back(std::move(points));
	}
	return true;
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::NotifyPropertyChanged(Platform::String ^ prop)
{

}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::Update(double elapsedTime)
{
	if (m_pOriSensor)
	{
		try
		{
			auto reading = m_pOriSensor->GetCurrentReading();
			auto rq = reading->Quaternion;
			DirectX::Quaternion q(rq->X, rq->Z, -rq->Y, rq->W);
			m_SurfaceOrientation = q;
		}
		catch (Platform::Exception^ exception) // Do nothing if the orientation reading failed
		{
			m_pOriSensor = Windows::Devices::Sensors::OrientationSensor::GetDefault();
		}
	}

	//XMMATRIX world = XMMatrixIdentity();
	XMVECTOR campos = m_SurfacePositon;

	XMVECTOR forward = XMVector3Rotate(g_XMNegIdentityR2.v, m_SurfaceOrientation);
	XMVECTOR up = XMVector3Rotate(g_XMIdentityR1.v, m_SurfaceOrientation);
	campos += forward * CameraDepth;

	XMMATRIX view = XMMatrixLookToRH(campos, forward, up);
	m_View = view;

	XMMATRIX viewProj = XMMatrixMultiply(view, m_Projection);
	XMVECTOR det;
	m_ScreenToWorld = XMMatrixInverse(&det,viewProj);

	//det = XMVectorSet(0, 0, -1.0f, 1.0f);
	//det = XMVector3TransformCoord(det, viewProj);
	////det = XMVectorSet(0, 0, -m_SliceDepthScreenSpace, 1.0f);
	//det = XMVector3TransformCoord(det, m_ScreenToWorld);
	//Panel::Update(elapsedTime);
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::Render()
{
	ClearPanel(DirectX::Colors::White.v);

	m_pDrawer->SetView(m_View);
	m_pDrawer->SetProjection(m_Projection);

	DrawAxis();

	DrawStrokes();

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
	//std::cout << "Lalala" << std::endl;
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::CreateSizeDependentResources()
{
	Panel::CreateSizeDependentResources();
	float aspect = _renderTargetWidth / _renderTargetHeight;
	m_ViewPortSize = Vector2(ActualWidth, ActualHeight);
	XMMATRIX proj = XMMatrixPerspectiveFovRH(XMConvertToRadians(75), aspect, 0.01f, 100.f);
	m_Projection = proj;
	XMVECTOR det = XMVectorSet(0, 0, -1.0f, 1.0f);
	det = XMVector3TransformCoord(det, proj);

	m_SliceDepthScreenSpace = XMVectorGetZ(det);
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
	m_pDrawer->SetWorld(XMMatrixIdentity());
	float ub = 5, lb = -5, majorIdent = 1, minorIdent = 0.25f;

	//m_pDrawer->DrawSphere({ .0f,.0f,.0f,.1f }, Colors::Cyan);

	float Ar = 0.03f, Al = ub, Almr = Al - Ar, Alpr = Al + Ar;
	m_pDrawer->Begin();

	for (float x = lb; x <= ub; x += minorIdent)
	{
		if (abs(fmod(x, majorIdent)) < 0.5f*minorIdent)
			continue;
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
	m_pDrawer->End();

	m_pDrawer->Begin();

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

void DimSketch::Xaml::Controls::Tablet3DViewPanel::DrawStrokes()
{
	//critical_section::scoped_lock lock(_criticalSection);

	m_pDrawer->SetWorld(XMMatrixIdentity());
	m_pDrawer->Begin();

	//m_pDrawer->GetDeviceContext()->RSSetState(m_pDrawer->GetStates()->CullClockwise());
	for (auto& curve : m_Strokes)
	{
		XMVECTOR v0 = curve[0];
		for (int i = 1; i < curve.size() - 1; i++)
		{
			m_pDrawer->DrawTriangle(v0,curve[i], curve[i + 1], Colors::Azure);
		}
	}
	m_pDrawer->End();

	m_pDrawer->Begin();
	for (auto& curve : m_Strokes)
	{
		for (int i = 0; i < curve.size() ; i++)
		{
			m_pDrawer->DrawLine(curve[i], curve[(i + 1)% curve.size()], Colors::LimeGreen);
		}
	}
	m_pDrawer->End();

}
