#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <string>
#include "application.h"
#include "simulator.h"
#include "engine.h"

char** getCommandLineArguments(int* argc)
{
	/* Get command line arguments in the wide character format. */
	LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), argc);

	/* Convert the arguments from the wide character to the multibyte format. */
	char** argv = new char*[*argc];
	for (int i = 0; i < *argc; i++) {
		int argSize = WideCharToMultiByte(CP_ACP, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
		argv[i] = new char[argSize];
		WideCharToMultiByte(CP_ACP, 0, wargv[i], -1, argv[i], argSize, nullptr, nullptr);
	}
	
	/* Free the memory that was allocated by CommandLineToArgvW. */
	LocalFree(wargv);

	return argv;
}

void freeCommandLineArguments(char** argv, int argc)
{
	for (int i = 0; i < argc; i++)
		delete argv[i];
	delete argv;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
	/* Default engine initialization values. */
	char* dllfile = nullptr;
	char** engineArgv = nullptr;
	int engineArgc = 0;

	/* Get command line arguments. */
	int argc = 0;
	char** argv = getCommandLineArguments(&argc);

	/* Try to find the -engine switch. */
	int engineIdx = 1;
	while (argv != nullptr && engineIdx < argc && (strcmp(argv[engineIdx], "-engine") != 0))
		engineIdx++;

	/* If the -engine switch was found. */
	if (engineIdx < argc) {
		/* And the -engine switch was not the last one. */
		if (argc - engineIdx >= 2) {
			/* The argument that follows is the DLL file path. */
			dllfile = argv[engineIdx + 1];
		}

		/* If at least two arguments follow the -engine switch. */
		if (argc - engineIdx >= 3) {
			/* Arguments were given for the engine. */
			engineArgv = &argv[engineIdx + 2];
			engineArgc = argc - engineIdx - 2;
		}
	}

	/* Load the engine. */
	bool engineLoaded = Engine::Initialize(dllfile);
	if (dllfile != nullptr && !engineLoaded) {
		std::string msg = std::string("Error loading engine ") + std::string(dllfile) + std::string("!");
		MessageBox(nullptr, msg.c_str(), "Engine error", MB_OK);
		return -1;
	}
	
	/* Determine the simulation parameters. */
	Engine::InitSimulatorParameters();
	Engine::simulatorParameters.argv = engineArgv;
	Engine::simulatorParameters.argc = engineArgc;
	Engine::simulatorInitialize(Engine::simulatorParameters);

	/* Construct the application. */
	Simulator* simulator = nullptr;

	switch (Engine::simulatorParameters.applicationType) {
	case Engine::UIType::GUI:
		{
			/* The GUI type of application. */
			if (!Application::InitializeGui(hInstance))
				return -1;

			Window* window = Application::createWindow(
				"Cart-pole simulator",
				Engine::simulatorParameters.windowWidth,
				Engine::simulatorParameters.windowHeight
			);

			if (!window->hasDrawingDevice()) {
				MessageBox(nullptr, "Cannot initialize DirectX!", "Direct2D error", MB_OK);
				return -1;
			}

			window->getDrawingDevice()->resetCamera();

			simulator = new Simulator();
			Application::assignSimulator(simulator);
			break;
		}

		case Engine::UIType::CONSOLE:
		{
			/* The CONSOLE type of application. */
			if (!Application::InitializeConsole(hInstance))
				return -1;

			simulator = new Simulator();
			Application::assignSimulator(simulator);

			std::cout << "Cart-pole simulator" << std::endl;
			std::cout << "Action frequency: " << Engine::simulatorParameters.actionFrequency << " Hz" << std::endl;
			std::cout << "Simulation speed: " << Engine::simulatorParameters.simulationSpeed << "x" << std::endl;
			std::cout << "Manual force: " << Engine::simulatorParameters.manualForce << " N" << std::endl;
			std::cout << "Mass: " << Engine::simulatorParameters.cart.mass << " Kg / ";
			std::cout << Engine::simulatorParameters.pole.mass << " Kg" << std::endl;
			std::cout << "Damping: " << Engine::simulatorParameters.cart.damping << std::endl;
			if (Engine::isLoaded()) {
				if (Engine::simulatorParameters.engineName != nullptr)
					std::cout << "Engine: " << Engine::simulatorParameters.engineName << std::endl;
				else
					std::cout << "Engine loaded" << std::endl;
			}
			else {
				std::cout << "No engine" << std::endl;
			}
			std::cout << std::endl;

			break;
		}
	}

	/* Run the application. */
	Application::setTimer(1.0 / (double)Engine::simulatorParameters.actionFrequency);
	Application::Run();

	/* Shutdown the application. */
	Engine::simulatorShutdown();	
	Application::saveLog();
	Engine::Destroy();

	if (simulator != nullptr)
		delete simulator;

	Application::Close();

	freeCommandLineArguments(argv, argc);

	return 0;
}