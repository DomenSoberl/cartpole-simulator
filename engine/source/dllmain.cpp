#include <Windows.h>
#include <string>
#include "engine.h"

#define DLLEXPORT extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    LPVOID lpReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

/* A text buffer used to send log entries to the simulator. */
char logBuffer[1024];

/* Used to send log entries to the simulator. The simulator
   will read and clear the buffer with every frame update. */
void Log(std::string s)
{
    strcat_s(logBuffer, 1024, (s).c_str());
}

/* Called when the simulator is started. */
DLLEXPORT void simulatorInitialize(SimulatorParameters& simulatorParameters)
{
    /* Tell the simulator where to find the log messages. */
    simulatorParameters.pLogBuffer = logBuffer;

    /* Tell the simulator where to save the log when the simulator shuts down. */
    simulatorParameters.logFilename = "log.txt"; // NULL if not saving.

    /* Parse the command line arguments. */
    if (simulatorParameters.argc > 0) {
        Log("Command line arguments:");
        char** pArg = simulatorParameters.argv;
        for (int i = 0; i < simulatorParameters.argc; i++)
            Log(std::string(" ") + std::string(pArg[i]));
        Log("\n");
    }

    /* Set the simulation properties. */
    simulatorParameters.applicationType = GUI;
    simulatorParameters.engineName = "Example";
    simulatorParameters.windowWidth = 1350;
    simulatorParameters.windowHeight = 800;
    simulatorParameters.actionFrequency = 50;
    simulatorParameters.simulationSpeed = 1;
    simulatorParameters.gravity = 9.81;
    simulatorParameters.manualForce = 10;

    /* Set the cart parameters. */
    simulatorParameters.cart.size = 1.0;
    simulatorParameters.cart.mass = 1.0;
    simulatorParameters.cart.damping = 0.5;
    simulatorParameters.pole.size = 1.0;
    simulatorParameters.pole.mass = 0.1;
    simulatorParameters.pole.damping = 0.5;
    
    /* Set the camera parameters. */
    simulatorParameters.camera.x = -6;
    simulatorParameters.camera.y = 0;
    simulatorParameters.camera.zoom = 0.9;

    /* Place craters or hills. */
    static const Crater craters[] = {
        {-6, 10, 1.5}, // {position, width, depth}
        {0, 0, 0} // Array must end with a zero-crater.
    };

    /* Place vertical markers. */
    static const Marker markers[] = {
        {0, 0.1, 192, 192, 64}, // {x, width, red, green, blue}
        {0, 0, 0, 0, 0} // Array must end with a zero-marker.
    };

    /* Set the carters and markers. */
    simulatorParameters.craters = craters;
    simulatorParameters.markers = markers;

    /* Print out a log message. */
    Log("Engine initialized.\n");
}

/* Called when the simulator is being shut down. */
DLLEXPORT void simulatorShutdown()
{
    Log("Shutting down the engine.\n");
}

/* Called when the simulation is being reset. */
DLLEXPORT void setInitialState(InitialState& initialState)
{
    /* Set the variables as they are supposed to be in the initial state.
       All the initial state values are preset to 0. */
    initialState.x = -12;

    Log("Simulation has been reset.\n");
}

/* Called every time the state changes. */
DLLEXPORT void stateUpdated(double simulationTime, SimulationState simulationState, SimulationParameters& simulationParameters)
{
    /* Process/store the current state. */

    /* Camera can be moved or zoomed. */
    simulationParameters.cameraParameters.x = simulationState.x;
    simulationParameters.cameraAction = UPDATE_CAMERA;

    /* Simulation can be reset. */
    if (simulationState.x >= 0) {
        simulationParameters.simulationAction = RESET_SIMULATION;
    }

    /* Simulation can be terminated. */
    if (simulationState.x < -15) {
        simulationParameters.simulationAction = TERMINATE_SIMULATION;
    }
}

/* Called when the simulator expects the engine to apply an action. */
DLLEXPORT void applyAction(CartAction& cartAction)
{   
    /* Set the force of the action in Newtons. */
    cartAction.force = 2.0;

    /*
        We can:
        - Apply no action (APPLY_NO_ACTION).
        - Apply the specified force (APPLY_FORCE / default).
        - Apply the manual action received through keyboard input (APPLY_MANUAL_ACTION).
        - Apply the specified force if there is no keyboard input, otherwise apply the
          manual action (APPLY_FORCE_IF_NO_MANUAL_ACTION).
    */
    cartAction.options = APPLY_FORCE_IF_NO_MANUAL_ACTION;
}

/* Called when user pressed a key. */
DLLEXPORT int keyPressed(KeyInfo& keyInfo)
{
    if (keyInfo.state == KeyState::PRESSED) {
        if (keyInfo.code == 0x20) { // Space key
            Log("Space bar has been pressed.\n");

            /* Return 1, if the key was processed (supress its default behavior). */
            return 1;
        }
    }

    /* Return 0, if the key was not processed (execute the default key behavior). */
    return 0;
}