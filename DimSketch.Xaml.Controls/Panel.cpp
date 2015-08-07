//------------------------------------------------------------------------------
// <copyright file="Panel.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "pch.h"
#include "Panel.h"
#include "DirectXHelper.h"
#include "shaders.h"
#include <DirectXColors.h>

using namespace Concurrency;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::System::Threading;
using namespace Windows::UI::Input;

using namespace DimSketch::Xaml::Controls::Base;

Panel::Panel()
    : DirectXPanel()
	, _muiltiSamplesCount(4)
    , _trackState(InputState::None)
    , _loadingComplete(false)
{
}

uint32 Panel::MuiltiSamplesCount::get()
{
	return _muiltiSamplesCount;
}

void Panel::MuiltiSamplesCount::set(uint32 msaaCount) 
{
	_muiltiSamplesCount = msaaCount;
}


void Panel::ClearPanel(_In_opt_ FXMVECTOR color)
{
	_renderTarget.Clear(_d3dContext.Get(), color);
	_renderTarget.SetAsRenderTarget(_d3dContext.Get());
}

/*
void Panel::RenderTexture(_In_ Texture^ renderTexture)
{
    RenderTextureToSize(renderTexture, static_cast<UINT>(_renderTargetWidth), static_cast<UINT>(_renderTargetHeight));
}

void Panel::RenderTextureToSize(_In_ Texture^ renderTexture, UINT targetWidth, UINT targetHeight)
{
    // update the buffer parameters
    RenderTextureEffect::Paramaters cbParams;
    ZeroMemory(&cbParams, sizeof(RenderTextureEffect::Paramaters));

    if (DXGI_FORMAT_G8R8_G8B8_UNORM == renderTexture->TextureFormat)
    {
        cbParams.texParams.x = SURFACE_TEXTURE_MODE_YUV;
    }
    else
    {
        cbParams.texParams.x = SURFACE_TEXTURE_MODE_NONE;
    }

    cbParams.texSize.x = static_cast<float>(renderTexture->Width);
    cbParams.texSize.y = static_cast<float>(renderTexture->Height);

    cbParams.ctlSize.x = static_cast<float>(targetWidth);
    cbParams.ctlSize.y = static_cast<float>(targetHeight);

    cbParams.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    // apply shader effect
    if (!_renderTextureEffect->Apply(_d3dContext.Get(), renderTexture->TextureSRV, &cbParams))
    {
        return;
    }

    RenderPanel();
}
void Panel::RenderPanel()
{
    // Set primitive topology
    _d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    // Draw the scene
    _d3dContext->Draw(1, 0);

    // reset
    ID3D11Buffer* buffer = nullptr;
    _d3dContext->GSSetConstantBuffers(0, 1, &buffer);
    _d3dContext->PSSetConstantBuffers(0, 1, &buffer);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    _d3dContext->PSSetShaderResources(0, 1, &nullSRV);
}
*/

void Panel::Present()
{
	if (_muiltiSamplesCount > 1)
		_d3dContext->ResolveSubresource(_backBuffer, 0, _renderTarget.ColorBuffer(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);
    // finalize the render
    DirectXPanel::Present();
}

void Panel::ResetDeviceResources()
{
    _loadingComplete = false;

	_renderTarget.Reset();
	_backBuffer.Reset();

    DirectXPanel::ResetDeviceResources();

    _renderTextureEffect = nullptr;
}

void Panel::CreateDeviceResources()
{
    DirectXPanel::CreateDeviceResources();

    // Retrieve DXGIOutput representing the main adapter output.
    ComPtr<IDXGIFactory1> dxgiFactory;
    ThrowIfFailed(
        CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))
        );

    ComPtr<IDXGIAdapter> dxgiAdapter;
    ThrowIfFailed(
        dxgiFactory->EnumAdapters(0, &dxgiAdapter)
        );

    ThrowIfFailed(
        dxgiAdapter->EnumOutputs(0, &_dxgiOutput)
        );

    // shader for rendering texture
    _renderTextureEffect = ref new RenderTextureEffect();
    _renderTextureEffect->Initialize(_d3dDevice.Get());
}

void Panel::CreateSizeDependentResources()
{
    // Clear the previous window size specific context.
	_backBuffer.Reset();
	_renderTarget.Reset();

    // base class create
    DirectXPanel::CreateSizeDependentResources();

    // Create a render target view of the swap chain back buffer.
	_backBuffer = RenderableTexture2D(_swapChain.Get());

    // Create depth/stencil buffer descriptor.
	auto depthBuffer = DepthStencilBuffer(_d3dDevice.Get(), (int)_renderTargetWidth, (int)_renderTargetHeight, DXGI_FORMAT_D24_UNORM_S8_UINT,_muiltiSamplesCount,0);

	RenderableTexture2D colorBuffer;

	// if not MSAA, direct render to back buffer
	if (_muiltiSamplesCount == 1)
		colorBuffer = _backBuffer;
	else
		colorBuffer = RenderableTexture2D(_d3dDevice.Get(), (int)_renderTargetWidth, (int)_renderTargetHeight, DXGI_FORMAT_B8G8R8A8_UNORM, _muiltiSamplesCount, 0);

	CD3D11_VIEWPORT viewPort(.0f,.0f, _renderTargetWidth, _renderTargetHeight);
	_renderTarget = RenderTarget(colorBuffer, depthBuffer, viewPort);

    _loadingComplete = true;
}

void Panel::StartRenderLoop()
{
    // start render loop
    DirectXPanel::StartRenderLoop();

    // setup touch input
    // Initialize the rendering surface and prepare it to receive input.
    _trackState = InputState::None;

    _activePointerId[0] = 0;
    _activePointerId[1] = 0;

    // Create a task to register for independent input and begin processing input messages.
    auto inputHandler = ref new WorkItemHandler([this](IAsyncAction ^)
    {
        // The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
        _coreInput = CreateCoreIndependentInputSource(
            Windows::UI::Core::CoreInputDeviceTypes::Mouse |
            Windows::UI::Core::CoreInputDeviceTypes::Touch |
            Windows::UI::Core::CoreInputDeviceTypes::Pen);

        // Register for pointer events, which will be raised on the background thread.
        _pointerPressedToken = _coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &Panel::OnPointerPressed);
        _pointerMovedToken = _coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &Panel::OnPointerMoved);
        _pointerReleasedToken = _coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &Panel::OnPointerReleased);
        _pointerWheelChangedToken = _coreInput->PointerWheelChanged += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &Panel::OnPointerWheelChanged);

        // Begin processing input messages as they're delivered.
        _coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);

        _coreInput->PointerPressed -= _pointerPressedToken;
        _coreInput->PointerMoved -= _pointerMovedToken;
        _coreInput->PointerReleased -= _pointerReleasedToken;
        _coreInput->PointerWheelChanged -= _pointerWheelChangedToken;
    });

    // Run task on a dedicated high priority background thread.
    _inputLoopWorker = ThreadPool::RunAsync(inputHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void Panel::StopRenderLoop()
{
    DirectXPanel::StopRenderLoop();

    _activePointerId[0] = 0;
    _activePointerId[1] = 0;

    // A call to ProcessEvents() with the ProcessUntilQuit flag will only return by default when the window closes.
    // Calling StopProcessEvents allows ProcessEvents to return even if the window isn't closing so the background thread can exit.
    _coreInput->Dispatcher->StopProcessEvents();
}

void Panel::OnPointerPressed(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e)
{
    critical_section::scoped_lock lock(_inputLock);

    XMFLOAT2 position = XMFLOAT2(
        e->CurrentPoint->Position.X,
        e->CurrentPoint->Position.Y);

    // Handle the PointerPressed event, which will be raised on a background thread.
    if (e->CurrentPoint->Properties->PointerUpdateKind == PointerUpdateKind::LeftButtonPressed
        && _trackState == InputState::None)
    {
        _trackState = InputState::Tracking;
        _previousPoint[0] = position;

        // Store active pointer ID: only care about this point
        _activePointerId[0] = e->CurrentPoint->PointerId;
    }
    else if (e->CurrentPoint->Properties->PointerUpdateKind == PointerUpdateKind::LeftButtonPressed
        && _trackState == InputState::Tracking)
    {
        _trackState = InputState::Zooming;
        _previousPoint[1] = position;
        _activePointerId[1] = e->CurrentPoint->PointerId;

        // calc distance from first point
        XMFLOAT2 sub = _previousPoint[1];
        sub.x = -_previousPoint[0].x;
        sub.y = -_previousPoint[0].y;

        _moveDistance = hypotf(sub.x, sub.y);
    }
}

void Panel::OnPointerMoved(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e)
{
    critical_section::scoped_lock lock(_inputLock);

    XMFLOAT2 position = XMFLOAT2(
        e->CurrentPoint->Position.X,
        e->CurrentPoint->Position.Y);

    // Handle the PointerMoved event, which will be raised on a background thread.
    if (_trackState == InputState::Tracking
        && e->CurrentPoint->PointerId == _activePointerId[0])
    {
        _previousPoint[0] = position;
    }
    else if (_trackState == InputState::Zooming)
    {
        if (e->CurrentPoint->PointerId == _activePointerId[0])
        {
            _previousPoint[0] = position;                  // save for computing relative delta
        }
        else if (e->CurrentPoint->PointerId == _activePointerId[1])
        {
            _previousPoint[1] = position;                  // save for computing relative delta
        }

        XMFLOAT2 sub = _previousPoint[1];
        sub.x = -_previousPoint[0].x;
        sub.y = -_previousPoint[0].y;

        float distance = hypotf(sub.x, sub.y);

        _deltaZoomChange = distance - _moveDistance;        // zoom in/out

        _moveDistance = distance;                           // save for next frame
    }
}

void Panel::OnPointerReleased(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e)
{
    critical_section::scoped_lock lock(_inputLock);

    if (e->CurrentPoint->Properties->PointerUpdateKind == PointerUpdateKind::RightButtonReleased)
    {
        // When right-clicks are unhandled on the background thread, the platform can use them for AppBar invocation.
        e->Handled = false;
    }
    else if (_trackState == InputState::Tracking)
    {
        _trackState = InputState::None;

        // Reset active pointer ID.
        _activePointerId[0] = _activePointerId[1];
    }
    else if (_trackState == InputState::Zooming)
    {
        _trackState = InputState::Tracking;
        _activePointerId[1] = 0;
    }
}

void Panel::OnPointerWheelChanged(Platform::Object^ sender, Windows::UI::Core::PointerEventArgs^ e)
{
    critical_section::scoped_lock lock(_inputLock);

    int delta = e->CurrentPoint->Properties->MouseWheelDelta;
}
