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
	BeginRender(nullptr, nullptr, DirectX::Colors::Azure.f);
	EndRender();
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::ResetDeviceResources()
{
	Panel::ResetDeviceResources();
}

void DimSketch::Xaml::Controls::Tablet3DViewPanel::CreateDeviceResources()
{
	Panel::CreateDeviceResources();
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
