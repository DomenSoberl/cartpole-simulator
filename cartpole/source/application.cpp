#include <iostream>
#include "application.h"
#include "cpuusage.h"
#include "resource.h"

Application::Type Application::type = Application::Type::GUI;
HINSTANCE Application::hInstance = nullptr;
Window* Application::window = nullptr;
Simulator* Application::simulator = nullptr;
Timer Application::timer;
double Application::dt = 0;

bool Application::InitializeGui(HINSTANCE hInstance)
{
	if (CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) != S_OK) {
		MessageBox(nullptr, "Cannot initialize COM Library!", "COM Error", MB_OK);
		return false;
	}

	Application::type = Type::GUI;
	Application::hInstance = hInstance;
	Application::window = nullptr;
	Application::simulator = nullptr;
	Window::hInstance = hInstance;
	Window::className = "Window";

	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = Application::WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = Application::hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CARTPOLE));
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = Window::className;
	wc.hIconSm = nullptr;

	RegisterClassEx(&wc);

	return true;
}

bool Application::InitializeConsole(HINSTANCE hInstance)
{
	Application::type = Type::CONSOLE;
	Application::hInstance = hInstance;
	Application::window = nullptr;
	Application::simulator = nullptr;

	if (AllocConsole() == 0)
		return false;

	FILE* console;
	freopen_s(&console, "CONOUT$", "w", stdout);

	return true;
}

LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window* window = nullptr;

	if (uMsg == WM_NCCREATE) {
		CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
		window = reinterpret_cast<Window*>(createStruct->lpCreateParams);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
		window->hwnd = hwnd;
	}
	else {
		window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (uMsg == WM_DESTROY && window != nullptr) {
		delete window;
		window = nullptr;
		PostQuitMessage(0);
	}

	if (window != nullptr)
		return window->WindowProc(uMsg, wParam, lParam);
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

Window* Application::createWindow(std::string windowName)
{
	if (window != nullptr)
		delete window;

	window = new Window(hInstance, windowName.c_str());
	
	return window;
}

Window* Application::createWindow(std::string windowName, int width, int height)
{
	if (window != nullptr)
		delete window;

	window = new Window(hInstance, windowName.c_str(), width, height);
	
	return window;
}

void Application::assignSimulator(Simulator* simulator)
{
	Application::simulator = simulator;
	
	if (window != nullptr)
		window->assignSimulator(simulator);
}

void Application::setTimer(double seconds)
{
	timer.setInterval(seconds);
	timer.start();
}

void Application::update()
{
	if (window != nullptr)
		window->update();

	if (Application::type == Application::Type::CONSOLE && Engine::simulatorParameters.pLogBuffer != nullptr)
		std::cout << Engine::simulatorParameters.pLogBuffer;

	Engine::ClearLogBuffer();
}

bool Application::priorityUpdate()
{
	if (simulator == nullptr)
		return false;

	return simulator->hasPriorityUpdate();
}

bool Application::terminationDemand()
{
	if (simulator == nullptr)
		return false;

	return simulator->wantsToTerminate();
}

void Application::sendTimerEvent()
{
	if (simulator != nullptr)
		simulator->tick(timer.getInterval());
}

void Application::Run()
{
	CPUUsage::setFrequency(Engine::simulatorParameters.actionFrequency);
	CPUUsage::clear();

	MSG msg;
	bool run = true;
	while (run) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				run = false;
		}

		if (priorityUpdate()) {
			sendTimerEvent();
			update();
			timer.reset();
		}
		else {
			/* When the timer triggers, it is time to draw the next frame. If the speed is higher than
			   1x, compute as many frames as the speed-up and paint only the last one. */
			if (timer.deadline()) {

				/* We may be several frames behind the deadline. If so, try to catch up. All frames are
				   processed, but only the last one is refreshed. This means that all the recordings are
				   correct, but frames may be skipped in the real-time animation. However, it may not be
				   possible to catch up with the deadline, if frame processing is consistently slow. This
				   could resolve in an endless loop and application freeze. We therefore break the loop
				   after 3 frames and readjust the timer. This means that all the frames still get
				   processed and recorded correctly, but we give up on real-time animation. */
				int frames = 0;
				while (timer.deadline() && frames < 3) {
					/* Increase the deadline for one timer interval. */
					timer.nextInterval();

					/* Compute frames. */
					for (int frames = 0; frames < Engine::simulatorParameters.simulationSpeed; frames++) {
						sendTimerEvent();
					}

					/* Compute the CPU usage - how long did the frame processing take? */
					CPUUsage::reportUsage(timer.timeFromDeadline() / timer.getInterval());

					frames++;
				}

				/* If more than 3 frames behind, set the timer one interval behind the current
				   time, so that it immediatelly triggers in the next iteration, but stays close
				   to the current time. If the CPU burden is later lowered, so that real-time
				   animation is again possible, this prevents it to speed up the animation
				   to catch up for the lost time, but rather continues in real-time. */
				if (frames >= 3)
					timer.catchUp();

				/* Paint the last frame in the loop. */
				update();
			}
		}

		if (terminationDemand())
			run = false;
	}
}

void Application::saveLog()
{
	if (Engine::simulatorParameters.logFilename == nullptr)
		return;

	if (simulator != nullptr) {
		simulator->updateLog();
		simulator->saveLog(Engine::simulatorParameters.logFilename);
		Engine::ClearLogBuffer();
	}
}

void Application::Close()
{
	switch (Application::type) {
	case Application::Type::GUI:
		CoUninitialize();
		break;
	case Application::Type::CONSOLE:
		FreeConsole();
		break;
	}
}