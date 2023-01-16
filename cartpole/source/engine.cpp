#include <string>
#include "engine.h"

Engine::FunctionSimulatorInitialize Engine::simulatorInitialize = Engine::defaultSimulatorInitialize;
Engine::FunctionSimulatorShutdown Engine::simulatorShutdown = Engine::defaultSimulatorShutdown;
Engine::FunctionSetInitialState Engine::setInitialState = Engine::defaultSetInitialState;
Engine::FunctionStateUpdated Engine::stateUpdated = Engine::defaultStateUpdated;
Engine::FunctionApplyAction Engine::applyAction = Engine::defaultApplyAction;
Engine::FunctionKeyPressed Engine::keyPressed = Engine::defaultKeyPressed;
Engine::SimulatorParameters Engine::simulatorParameters;
HMODULE	Engine::dll = nullptr;

bool Engine::Initialize(const char* dllfile)
{
	if (dll != nullptr)
		return false;

	if (dllfile == nullptr || *dllfile == 0)
		dll = LoadLibrary("cartpole.dll");
	else
		dll = LoadLibrary(dllfile);

	if (dll != nullptr) {
		Engine::simulatorInitialize =
			(Engine::FunctionSimulatorInitialize)GetProcAddress(dll, "simulatorInitialize");
		if (Engine::simulatorInitialize == nullptr)
			Engine::simulatorInitialize = Engine::defaultSimulatorInitialize;

		Engine::simulatorShutdown =
			(Engine::FunctionSimulatorShutdown)GetProcAddress(dll, "simulatorShutdown");
		if (Engine::simulatorShutdown == nullptr)
			Engine::simulatorShutdown = Engine::defaultSimulatorShutdown;

		Engine::setInitialState =
			(Engine::FunctionSetInitialState)GetProcAddress(dll, "setInitialState");
		if (Engine::setInitialState == nullptr)
			Engine::setInitialState = Engine::defaultSetInitialState;

		Engine::stateUpdated =
			(Engine::FunctionStateUpdated)GetProcAddress(dll, "stateUpdated");
		if (Engine::stateUpdated == nullptr)
			Engine::stateUpdated = Engine::defaultStateUpdated;

		Engine::applyAction =
			(Engine::FunctionApplyAction)GetProcAddress(dll, "applyAction");
		if (Engine::applyAction == nullptr)
			Engine::applyAction = Engine::defaultApplyAction;

		Engine::keyPressed =
			(Engine::FunctionKeyPressed)GetProcAddress(dll, "keyPressed");
		if (Engine::keyPressed == nullptr)
			Engine::keyPressed = Engine::defaultKeyPressed;
	}

	return (dll != nullptr);
}

void Engine::Destroy()
{
	if (dll != nullptr) {
		FreeLibrary(dll);
		dll = nullptr;
	}
}

void Engine::InitSimulatorParameters(SimulatorParameters* simulatorParameters)
{
	simulatorParameters->argv = nullptr;
	simulatorParameters->argc = 0;
	simulatorParameters->applicationType = UIType::GUI;
	simulatorParameters->engineName = nullptr;
	simulatorParameters->windowWidth = 1000;
	simulatorParameters->windowHeight = 300;
	simulatorParameters->actionFrequency = 50;
	simulatorParameters->simulationSpeed = 1;
	simulatorParameters->gravity = 9.81;
	simulatorParameters->manualForce = 10.0;
	simulatorParameters->cart = { 1.0, 1.0, 0.5 };
	simulatorParameters->pole = { 1.0, 0.1, 0.5 };
	simulatorParameters->camera = { 0.0, 0.0, 1.0 };
	simulatorParameters->craters = nullptr;
	simulatorParameters->markers = nullptr;
	simulatorParameters->pLogBuffer = nullptr;
	simulatorParameters->logFilename = nullptr;
}

void Engine::ClearLogBuffer()
{
	if (Engine::simulatorParameters.pLogBuffer != nullptr)
		*Engine::simulatorParameters.pLogBuffer = 0;
}

void Engine::defaultSimulatorInitialize(SimulatorParameters& simulatorParameters)
{
}

void Engine::defaultSimulatorShutdown()
{
}

void Engine::defaultSetInitialState(InitialState& initialState)
{
}

void Engine::defaultStateUpdated(double simulationTime, SimulationState simulationState, SimulationParameters& simulationParameters)
{
}

void Engine::defaultApplyAction(CartAction& cartAction)
{
	cartAction.force = 0;
	cartAction.options = ActionOptions::APPLY_MANUAL_ACTION;
}

int Engine::defaultKeyPressed(KeyInfo& keyInfo)
{
	return 0;
}