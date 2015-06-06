#pragma once

#include "Panel.h"
#include "PrimitiveVisualizer.h"

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
						{ return m_CameraDepth; }
						void set(float value)
						{ m_CameraDepth = value;}
					}

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
				private:
					std::shared_ptr<DirectX::PrimitveDrawer>	 m_pDrawer;
					float										 m_CameraDepth;
					DirectX::XMMATRIX							 m_View;
					DirectX::XMMATRIX							 m_Projection;
				};
		}
	}
}
