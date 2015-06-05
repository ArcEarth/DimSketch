#pragma once

#include "Panel.h"

namespace DimSketch {
	namespace Xaml {
		namespace Controls {
				using namespace DimSketch::Xaml::Controls::Base;

				public ref class Tablet3DViewPanel sealed
					: public Panel
				{
				public:
					Tablet3DViewPanel();

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

				private:

				};
		}
	}
}
