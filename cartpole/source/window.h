#pragma once
#include <windows.h>
#include "drawingdevice.h"
#include "simulator.h"
#include "timer.h"

class Window
{
	friend class Application;

public:
	int hasDrawingDevice() const { return drawingDevice != nullptr; }
	DrawingDevice* getDrawingDevice() const { return drawingDevice; }
	void assignSimulator(Simulator* simulator);

protected:
	Window(
		HINSTANCE hInstance,
		LPCTSTR windowName,
		int width = CW_USEDEFAULT,
		int height = CW_USEDEFAULT
	);
	~Window();

	void paint();
	void update();

private:
	static HINSTANCE hInstance;
	static LPCSTR className;
	HWND hwnd;
	DrawingDevice* drawingDevice;
	Simulator* simulator;
	int grabbedObject;
	double holdx;
	double holdy;

	LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};
