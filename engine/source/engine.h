#pragma once

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

typedef struct {
	double size;
	double mass;
	double damping;
} ObjectParameters;

typedef struct {
	double x;
	double y;
	double zoom;
} CameraParameters;

typedef struct {
	double x;
	double width;
	unsigned char red;
	unsigned char green;
	unsigned char blue;
} Marker;

typedef struct {
	double x;
	double width;
	double depth;
} Crater;

typedef struct {
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
} SimulatorParameters;

typedef struct {
	CameraParameters cameraParameters;
	CameraAction cameraAction;
	SimulationAction simulationAction;
} SimulationParameters;

typedef struct {
	double x;
	double dx;
	double ddx;
	double theta;
	double dtheta;
	double ddtheta;
} InitialState;

typedef struct {
	double x;
	double y;
	double phi;
	double dx;
	double ddx;
	double theta;
	double dtheta;
	double ddtheta;
} SimulationState;

typedef struct {
	double force;
	ActionOptions options;
} CartAction;

typedef struct {
	unsigned int code;
	char character;
	KeyState state;
} KeyInfo;