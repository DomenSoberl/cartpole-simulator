#pragma once
#include <windows.h>
#include <string>
#include "window.h"
#include "simulator.h"
#include "timer.h"

class Application
{
public:
	enum Type {
		GUI = 0,
		CONSOLE = 1
	};	

	static bool InitializeGui(HINSTANCE hInstance);
	static bool InitializeConsole(HINSTANCE hInstance);
	static Window* createWindow(std::string windowName);
	static Window* createWindow(std::string windowName, int width, int height);
	static void assignSimulator(Simulator* simulator);
	static void setTimer(double seconds);
	static void Run();
	static void saveLog();
	static void Close();

protected:
	static Type type;
	static HINSTANCE hInstance;
	static Window* window;
	static Simulator* simulator;
	static Timer timer;
	static double dt;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	static void update();
	static bool priorityUpdate();
	static bool terminationDemand();
	static void sendTimerEvent();
};
