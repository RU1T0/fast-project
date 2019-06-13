#pragma once

#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <assert.h>
#include <memory>
#include <iostream>

#include "misc.h"
#include "high_resolution_timer.h"
#include <d3d11.h>
#include "sprite.h"


const LONG SCREEN_WIDTH = 1280;
const LONG SCREEN_HEIGHT = 720;

class framework
{
public:
	const HWND hwnd;
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ID3D11Texture2D* tex2d;
	IDXGISwapChain* swapchain;
	ID3D11RenderTargetView* render_target_view;
	ID3D11DepthStencilView* depth_stncil_view;

	ID3D11BlendState*		pBlendState;

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	Sprite *sprite[1024];
	std::unique_ptr<Sprite> particle;
	std::unique_ptr<Sprite> text;

	benchmark bmark;

	float angle;

	framework(HWND hwnd) : hwnd(hwnd)
	{

	}
	~framework()
	{
		if (deviceContext)deviceContext->ClearState();

		if (render_target_view)render_target_view->Release();
		if (swapchain)swapchain->Release();
		if (deviceContext)deviceContext->Release();
		if (device)device->Release();
		if (depth_stncil_view)depth_stncil_view->Release();
		if (tex2d)tex2d->Release();
		if (pBlendState)pBlendState->Release();

		for (auto& p:sprite)
		{
			if (p)
				delete p;
		}
	}
	int run()
	{
		MSG msg = {};

		if (!initialize(hwnd)) return 0;
		timer.reset();

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				timer.tick();
				calculate_frame_stats();
				update(timer.time_interval());
				render(timer.time_interval());
			}
		}
		return static_cast<int>(msg.wParam);
	}

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			break;
		case WM_KEYDOWN:
			if (wparam == VK_ESCAPE) PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case WM_ENTERSIZEMOVE:
			// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
			timer.stop();
			break;
		case WM_EXITSIZEMOVE:
			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
			timer.start();
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

private:
	bool initialize(HWND);
	void update(float elapsed_time/*Elapsed seconds from last frame*/);
	void render(float elapsed_time/*Elapsed seconds from last frame*/);

private:
	high_resolution_timer timer;
	void calculate_frame_stats()
	{
		// Code computes the average frames per second, and also the 
		// average time it takes to render one frame.  These stats 
		// are appended to the window caption bar.
		static int frames = 0;
		static float time_tlapsed = 0.0f;

		frames++;

		// Compute averages over one second period.
		if ((timer.time_stamp() - time_tlapsed) >= 1.0f)
		{
			float fps = static_cast<float>(frames); // fps = frameCnt / 1
			float mspf = 1000.0f / fps;
			std::ostringstream outs;
			outs.precision(6);
			outs << "FPS : " << fps << " / " << "Frame Time : " << mspf << " (ms)";
			SetWindowTextA(hwnd, outs.str().c_str());

			// Reset for next average.
			frames = 0;
			time_tlapsed += 1.0f;
		}
	}
};

