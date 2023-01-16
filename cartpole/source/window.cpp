#include <windowsx.h>
#include "window.h"
#include "engine.h"

HINSTANCE Window::hInstance = nullptr;
LPCSTR Window::className = "";

Window::Window(HINSTANCE hInstance, LPCTSTR windowName, int width, int height) :
	hwnd(nullptr),
	drawingDevice(nullptr),
	simulator(nullptr),
	grabbedObject(-1),
	holdx(0),
	holdy(0)
{
	HWND hwnd = CreateWindowEx(
		0,
		Window::className,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		nullptr,
		nullptr,
		Window::hInstance,
		this
	);

	ShowWindow(hwnd, SW_SHOW);

	drawingDevice = new DrawingDevice(hwnd);
	if (!drawingDevice->isInitialized()) {
		delete drawingDevice;
		drawingDevice = nullptr;
	}
}

Window::~Window()
{
	KillTimer(hwnd, 0);
	if (drawingDevice != nullptr)
		delete drawingDevice;
}

void Window::assignSimulator(Simulator* simulator)
{
	this->simulator = simulator;
}

void Window::paint()
{
	if (drawingDevice == nullptr) return;

	drawingDevice->beginDraw();
	if (simulator != nullptr)
		simulator->paint(drawingDevice);
	drawingDevice->endDraw();
}

void Window::update()
{
	if (simulator != nullptr)
		simulator->updateLog();
	
	InvalidateRect(hwnd, nullptr, FALSE);
}

LRESULT Window::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_PAINT:
	{
		if (simulator == nullptr)
			break;

		paint();
		ValidateRect(hwnd, nullptr);
		break;
	}

	case WM_SIZE:
	case WM_SIZING:
	{
		if (drawingDevice != nullptr)
			drawingDevice->resize();
		InvalidateRect(hwnd, nullptr, FALSE);
		break;
	}

	case WM_LBUTTONDOWN:
	{
		if (drawingDevice == nullptr || simulator == nullptr)
			break;

		holdx = drawingDevice->s2wx(static_cast<double>(GET_X_LPARAM(lParam)));
		holdy = drawingDevice->s2wy(static_cast<double>(GET_Y_LPARAM(lParam)));
		grabbedObject = simulator->getObjectAt(holdx, holdy);
		simulator->freezeObject(grabbedObject, true);
		break;
	}

	case WM_LBUTTONUP:
	{
		simulator->freezeObject(grabbedObject, false);
		grabbedObject = -1;
		break;
	}

	case WM_MOUSEMOVE:
	{
		if (drawingDevice == nullptr || simulator == nullptr) break;
		double pointx = drawingDevice->s2wx(static_cast<double>(GET_X_LPARAM(lParam)));
		double pointy = drawingDevice->s2wy(static_cast<double>(GET_Y_LPARAM(lParam)));

		// Moving the ground
		if (grabbedObject == 0) {
			drawingDevice->moveCamera(holdx - pointx, holdy - pointy);
		}

		// Moving an object
		else if (grabbedObject > 0) {
			simulator->moveObjectBy(grabbedObject, pointx - holdx, pointy - holdy);
			holdx = pointx;
			holdy = pointy;
		}
		break;
	}

	case WM_MOUSEWHEEL:
	{
		double zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		if (zDelta == 0) break;
		if (drawingDevice != nullptr) {
			float factor = (zDelta > 0 ? 1.1f : 0.9f);
			drawingDevice->zoomIn(factor);
			InvalidateRect(hwnd, nullptr, FALSE);
		}
		break;
	}

	case WM_KEYDOWN:
	{
		if (drawingDevice == nullptr || simulator == nullptr) break;

		Engine::KeyInfo keyInfo;
		keyInfo.code = static_cast<unsigned int>(wParam);
		keyInfo.character = static_cast<char>(MapVirtualKeyA(keyInfo.code, MAPVK_VK_TO_CHAR));
		keyInfo.state = Engine::KeyState::PRESSED;
		if (Engine::keyPressed(keyInfo) != 0) break;

		switch (wParam) {
		case VK_RETURN:
			simulator->reset();
			break;
		case VK_LEFT:
			simulator->suppressEngineActions(true);
			simulator->setManualAction(-1);
			break;
		case VK_RIGHT:
			simulator->suppressEngineActions(true);
			simulator->setManualAction(1);
			break;
		case VK_UP:
			Engine::simulatorParameters.manualForce++;
			break;
		case VK_DOWN:
			if (Engine::simulatorParameters.manualForce > 0)
				Engine::simulatorParameters.manualForce--;
			break;
		case VK_F1:
			simulator->togglehelp();
			break;
		case VK_F2:
			simulator->toggleInfo();
			break;
		case VK_F3:
			simulator->toggleLog();
			break;
		case VK_F4:
			simulator->toggleCameraFrame();
			break;
		case VK_F5:
			drawingDevice->resetCamera();
			break;
		case VK_F6:
			simulator->startStopRecording();
			break;
		case VK_ESCAPE:
			simulator->cancel();
			break;
		}
		break;
	}

	case WM_KEYUP:
	{
		if (simulator == nullptr) break;

		Engine::KeyInfo keyInfo;
		keyInfo.code = static_cast<unsigned int>(wParam);
		keyInfo.state = Engine::KeyState::UNPRESSED;
		if (Engine::keyPressed(keyInfo) != 0) break;

		switch (wParam) {
		case VK_LEFT:
		case VK_RIGHT:
			simulator->suppressEngineActions(false);
			simulator->setManualAction(0);
			break;
		}
		break;
	}

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	return 0;
}