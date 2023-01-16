#pragma once
#include <string>
#include <vector>
#include "engine.h"
#include "drawingDevice.h"
#include "cart.h"
#include "recording.h"

class Simulator
{
public:
	Simulator();
	~Simulator();

	bool hasPriorityUpdate() const { return priorityUpdate; }
	bool wantsToTerminate()  const { return terminate; }
	bool isMouseOverLog(int x, int y, DrawingDevice* drawingDevice);
	int getObjectAt(double x, double y);
	void moveObjectBy(int object, double dx, double dy);
	void freezeObject(int object, bool freeze);
	void toggleInfo();
	void toggleLog();
	void togglehelp();
	void toggleCameraFrame();
	void getState(Engine::SimulationState& state);
	void startStopRecording();
	void cancel();
	void reset();
	void suppressEngineActions(bool suppress) { engineActionsSuppressed = suppress; }
	void setManualAction(double direction);
	void tick(double dt);
	void paint(DrawingDevice* drawingDevice);
	void updateLog();
	void saveLog(const char *filename);
	
protected:
	static const char helpText[];
	std::vector<DrawingDevice::Bezier> floor;
	bool priorityUpdate;
	bool terminate;
	bool frozen;
	Cart cart;
	bool engineActionsSuppressed;
	double simulationTime;
	double manualAction;
	double lastAction;
	bool showInfo;
	bool showLog;
	bool showHelp;
	bool showCameraFrame;
	double cameraX;
	double cameraY;
	double cameraZoom;
	bool updateCamera;
	Recording* recording;
	DrawingDevice* frameDrawingDevice;
	Cart* frameCart;
	std::string log;

private:
	void processRecording();
	void paintScenery(DrawingDevice* drawingDevice);
	void computeFloor();
	void alignCartWithFloor();
	double computeCartAngle(double x, double r, double epsilon, double& ycorrection);
	double getFloorHeight(double x);
	int computeBezierY(DrawingDevice::Point &point, DrawingDevice::Bezier& bezier, double x, double &y);
};