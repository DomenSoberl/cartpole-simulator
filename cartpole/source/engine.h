#pragma once
#include <Windows.h>

class Engine
{
public:
	enum UIType {
		GUI = 0,
		CONSOLE = 1
	};
	
	enum ActionOptions {
		APPLY_NO_ACTION = 0,
		APPLY_FORCE = 1,
		APPLY_MANUAL_ACTION = 2,
		APPLY_FORCE_IF_NO_MANUAL_ACTION = 3
	};

	enum SimulationAction {
		NO_SIMULATION_ACTION = 0,
		RESET_SIMULATION = 1,
		TERMINATE_SIMULATION = 2
	};

	enum CameraAction {
		NO_CAMERA_ACTION = 0,
		UPDATE_CAMERA = 1
	};

	enum KeyState {
		UNPRESSED = 0,
		PRESSED = 1
	};

	struct ObjectParameters {
		double size;
		double mass;
		double damping;
	};

	struct CameraParameters {
		double x;
		double y;
		double zoom;
	};

	struct Marker {
		double x;
		double width;
		unsigned char red;
		unsigned char green;
		unsigned char blue;
	};

	struct Crater {
		double x;
		double width;
		double depth;
	};

	struct SimulatorParameters {
		char** argv;
		int argc;
		UIType applicationType;
		const char* engineName;
		int windowWidth;
		int windowHeight;
		int actionFrequency;
		int simulationSpeed;
		double gravity;
		double manualForce;
		ObjectParameters cart;
		ObjectParameters pole;
		CameraParameters camera;
		const Crater* craters;
		const Marker* markers;
		char* pLogBuffer;
		const char* logFilename;
	};

	struct SimulationParameters {
		CameraParameters cameraParameters;
		CameraAction cameraAction;
		SimulationAction simulationAction;
	};

	struct InitialState {
		double x;
		double dx;
		double ddx;
		double theta;
		double dtheta;
		double ddtheta;
	};

	struct SimulationState {
		double x;
		double y;
		double phi;
		double dx;
		double ddx;
		double theta;
		double dtheta;
		double ddtheta;
	};

	struct CartAction {
		double force;
		Engine::ActionOptions options;
	};

	struct KeyInfo {
		unsigned int code;
		char character;
		Engine::KeyState state;
	};

	typedef void (__cdecl* FunctionSimulatorInitialize)(SimulatorParameters&);
	typedef void (__cdecl* FunctionSimulatorShutdown)();
	typedef void (__cdecl* FunctionSetInitialState)(InitialState&);
	typedef void (__cdecl* FunctionStateUpdated)(double, SimulationState, SimulationParameters&);
	typedef void (__cdecl* FunctionApplyAction)(CartAction&);
	typedef int (__cdecl* FunctionKeyPressed)(KeyInfo&);

	static bool Initialize(const char* dllfile);
	static void Destroy();
	static bool isLoaded() { return dll != nullptr; }
	static void InitSimulatorParameters(SimulatorParameters* simulatorParameters = &Engine::simulatorParameters);
	static void ClearLogBuffer();

	static SimulatorParameters simulatorParameters;
	static FunctionSimulatorInitialize simulatorInitialize;
	static FunctionSimulatorShutdown simulatorShutdown;
	static FunctionSetInitialState setInitialState;
	static FunctionStateUpdated stateUpdated;
	static FunctionApplyAction applyAction;
	static FunctionKeyPressed keyPressed;

protected:
	static void __cdecl defaultSimulatorInitialize(SimulatorParameters& simulatorParameters);
	static void __cdecl defaultSimulatorShutdown();
	static void __cdecl defaultSetInitialState(InitialState& initialState);
	static void __cdecl defaultStateUpdated(double simulationTime, SimulationState simulationState, SimulationParameters& simulationParameters);
	static void __cdecl defaultApplyAction(CartAction& cartAction);
	static int __cdecl defaultKeyPressed(KeyInfo& keyInfo);

private:
	static HMODULE dll;
};