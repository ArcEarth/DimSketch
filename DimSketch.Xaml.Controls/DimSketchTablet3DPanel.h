#pragma once

#include "Panel.h"
#include "PrimitiveVisualizer.h"

using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Input::Inking;

namespace DimSketch {
	namespace Xaml {
		namespace Controls {
			namespace Base
			{
				struct CameraParam
				{
					XMVECTOR Position;
					XMVECTOR Orientation;
				};
			}
				using namespace DimSketch::Xaml::Controls::Base;

				public ref class Tablet3DViewPanel sealed
					: public Panel
				{
				public:
					Tablet3DViewPanel();

					property float CameraDepth { 
						float get() 
						{ 
							return m_ZSliderDepth;
						}
						void set(float value)
						{
							m_ZSliderDepth = value;
						}
					}

					bool AddDrawStroke(IVectorView<InkStroke^>^ strokes);

				protected private:
					virtual event Windows::UI::Xaml::Data::PropertyChangedEventHandler^ PropertyChanged;
					void NotifyPropertyChanged(Platform::String^ prop);

					virtual void Update(double elapsedTime) override;
					virtual void Render() override;

					virtual void ResetDeviceResources() override;
					virtual void CreateDeviceResources() override;
					virtual void CreateSizeDependentResources() override;

					// override to add mouse/touch/pen input
					virtual void StartRenderLoop() override;
					virtual void StopRenderLoop() override;

				private:
					~Tablet3DViewPanel();

					void DrawAxis();
					void DrawStrokes();
				private:
					Windows::Devices::Sensors::OrientationSensor^	m_pOriSensor;
					std::shared_ptr<DirectX::PrimitveDrawer>		m_pDrawer;
					std::vector<std::vector<Vector3>>				m_Strokes;

					float											m_SliceDepthScreenSpace;
					float											m_ZSliderDepth;
					DirectX::Vector2								m_ViewPortSize;
					DirectX::Vector3								m_SurfacePositon;
					DirectX::Quaternion								m_SurfaceOrientation;
					DirectX::Matrix4x4								m_ScreenToWorld;
					DirectX::Matrix4x4								m_View;
					DirectX::Matrix4x4								m_Projection;
				};
		}
	}
}
