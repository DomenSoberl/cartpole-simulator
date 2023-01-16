#include <math.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include "simulator.h"
#include "cpuusage.h"

const char Simulator::helpText[] = "\
\n  F1     - show/hide help\
\n  F2     - show/hide information\
\n  F3     - show/hide engine log\
\n  F4     - show/hide camera frame\
\n  F5     - reset view\
\n  F6     - start/stop recording\
\n  Arrows - apply/change force\
\n  Enter  - reset simulation\
\n  Mouse  - move objects, change view\
";

Simulator::Simulator()
{
	priorityUpdate = false;
	terminate = false;
	frozen = false;
	engineActionsSuppressed = false;
	simulationTime = 0;
	manualAction = 0;
	lastAction = 0;
	showInfo = true;
	showLog = true;
	showHelp = false;
	showCameraFrame = false;
	cameraX = 0;
	cameraY = 0;
	cameraZoom = 1;
	updateCamera = false;
	recording = nullptr;
	frameDrawingDevice = nullptr;
	frameCart = nullptr;
	computeFloor();
	alignCartWithFloor();
	log = "";

	reset();
}

Simulator::~Simulator()
{
	if (recording != nullptr)
		delete recording;

	if (frameDrawingDevice != nullptr)
		delete frameDrawingDevice;

	if (frameCart != nullptr)
		delete frameCart;
}

bool Simulator::isMouseOverLog(int x, int y, DrawingDevice* drawingDevice)
{
	if (!showLog)
		return false;

	double width = drawingDevice->getWidth();
	double height = drawingDevice->getHeight();

	if (x < 2 * width / 3 || x > width)
		return false;
	if (y < 0 || y > height)
		return false;
	
	return true;
}

int Simulator::getObjectAt(double x, double y)
{
	return (!cart.frozen && cart.isTouched(x, y) ? 1 : 0);
}

void Simulator::moveObjectBy(int object, double dx, double dy)
{
	switch (object) {
	case 1:
		cart.x += dx;
		cart.y += dy;
		alignCartWithFloor();
		break;
	}
}

void Simulator::freezeObject(int object, bool freeze)
{
	switch (object) {
	case 1:
		cart.dropMomentum();
		cart.frozen = freeze;
		break;
	}
}

void Simulator::toggleInfo()
{
	showInfo = !showInfo;
}

void Simulator::toggleLog()
{
	showLog = !showLog;
}

void Simulator::togglehelp()
{
	showHelp = !showHelp;
}

void Simulator::toggleCameraFrame()
{
	showCameraFrame = !showCameraFrame;
}

void Simulator::getState(Engine::SimulationState& state)
{
	state.x = cart.x;
	state.y = cart.y;
	state.dx = cart.dx;
	state.ddx = cart.ddx;
	state.theta = cart.theta;
	state.dtheta = cart.dtheta;
	state.ddtheta = cart.ddtheta;
	state.phi = cart.phi;
}

void Simulator::startStopRecording()
{
	if (recording == nullptr)
		recording = new Recording(Engine::simulatorParameters.actionFrequency);
	else if (recording->state == Recording::State::RECORDING)
		recording->state = Recording::State::STOPPED;
}

void Simulator::cancel()
{
	if (recording != nullptr) {
		recording->state = Recording::State::FINISHED;
	}
}

void Simulator::reset()
{
	Engine::InitialState initialState;
	initialState.x = 0;
	initialState.dx = 0;
	initialState.ddx = 0;
	initialState.theta = 0;
	initialState.dtheta = 0;
	initialState.ddtheta = 0;

	Engine::setInitialState(initialState);

	simulationTime = 0;
	manualAction = 0;
	lastAction = 0;

	cart.reset(initialState.x, initialState.dx, initialState.ddx,
		initialState.theta, initialState.dtheta, initialState.ddtheta);
	alignCartWithFloor();
	
	Engine::SimulationState simulationState;
	getState(simulationState);
	
	Engine::SimulationParameters simulationParameters;
	simulationParameters.cameraParameters.x = cameraX;
	simulationParameters.cameraParameters.y = cameraY;
	simulationParameters.cameraParameters.zoom = cameraZoom;
	simulationParameters.cameraAction = Engine::CameraAction::NO_CAMERA_ACTION;
	simulationParameters.simulationAction = Engine::SimulationAction::NO_SIMULATION_ACTION;

	Engine::stateUpdated(simulationTime, simulationState, simulationParameters);

	switch (simulationParameters.cameraAction) {
	case Engine::CameraAction::UPDATE_CAMERA:
		cameraX = simulationParameters.cameraParameters.x;
		cameraY = simulationParameters.cameraParameters.y;
		cameraZoom = simulationParameters.cameraParameters.zoom;
		updateCamera = true;
		break;
	default:
		updateCamera = false;
	}

	switch (simulationParameters.simulationAction) {
	case Engine::SimulationAction::RESET_SIMULATION:
		/* The reset action is ignored, because the simulation has just been reset. */
		break;
	case Engine::SimulationAction::TERMINATE_SIMULATION:
		terminate = true;
		break;
	default:
		break;
	}
}

void Simulator::setManualAction(double direction)
{
	manualAction = direction * Engine::simulatorParameters.manualForce;
}

void Simulator::tick(double dt)
{
	/* If simulation is frozen, only process the recording. */
	if (frozen) {
		if (recording != nullptr)
			processRecording();
		return;
	}

	/* Ask the engine what action to execute. */
	Engine::CartAction cartAction;
	cartAction.force = 0;
	cartAction.options = Engine::ActionOptions::APPLY_FORCE;
	Engine::applyAction(cartAction);

	double action = 0;
	switch (cartAction.options) {
	case Engine::ActionOptions::APPLY_FORCE:
		action = cartAction.force;
		break;
	case Engine::ActionOptions::APPLY_MANUAL_ACTION:
		action = manualAction;
		break;
	case Engine::ActionOptions::APPLY_FORCE_IF_NO_MANUAL_ACTION:
		if (manualAction != 0)
			action = manualAction;
		else
			action = cartAction.force;
		break;
	default:
		break;
	}

	/* Update the simulation time. */
	simulationTime += dt;
	if (recording != nullptr)
		recording->time += dt;

	/* Simulate the cart. */
	cart.tick(action, dt);
	alignCartWithFloor();

	/* Prevent the cart from falling over the edge. */
	double leftBound = -100 + cart.getWidth() / 2;
	double rightBound = 100 - cart.getWidth() / 2;
	if (cart.x < leftBound || cart.x > rightBound) {
		if (cart.x < leftBound) cart.x = leftBound;
		if (cart.x > rightBound) cart.x = rightBound;
		cart.bounce();
	}

	/* Notify the engine that the state has been updated. */
	Engine::SimulationState simulationState;
	getState(simulationState);

	Engine::SimulationParameters simulationParameters;
	simulationParameters.cameraParameters.x = cameraX;
	simulationParameters.cameraParameters.y = cameraY;
	simulationParameters.cameraParameters.zoom = cameraZoom;
	simulationParameters.cameraAction = Engine::CameraAction::NO_CAMERA_ACTION;
	simulationParameters.simulationAction = Engine::SimulationAction::NO_SIMULATION_ACTION;

	Engine::stateUpdated(simulationTime, simulationState, simulationParameters);

	switch (simulationParameters.cameraAction) {
	case Engine::CameraAction::UPDATE_CAMERA:
		cameraX = simulationParameters.cameraParameters.x;
		cameraY = simulationParameters.cameraParameters.y;
		cameraZoom = simulationParameters.cameraParameters.zoom;
		updateCamera = true;
		break;
	default:
		updateCamera = false;
	}

	/* Does the engine want to change the simulation flow? */
	switch (simulationParameters.simulationAction) {
	case Engine::SimulationAction::RESET_SIMULATION:
		reset();
		break;
	case Engine::SimulationAction::TERMINATE_SIMULATION:
		terminate = true;
		break;
	default:
		break;
	}

	/* If recording, record the current state. */
	if (recording != nullptr) {
		processRecording();
	}
}

void Simulator::processRecording()
{
	switch (recording->state) {
	case Recording::State::RECORDING:
	{
		recording->snap(
			lastAction,
			cart.x,
			cart.y,
			cart.theta,
			cart.phi,
			cart.dx,
			cart.ddx,
			cart.dtheta,
			cart.ddtheta,
			cameraX,
			cameraY,
			cameraZoom
		);
		break;
	}
	case Recording::State::STOPPED:
	{
		frozen = true;
		recording->createFolder();
		recording->saveFramesData();
		recording->savedFrames = 0;
		frameDrawingDevice = new DrawingDevice(1280, 720);
		frameCart = new Cart();
		recording->state = Recording::State::PROCESSING;
		priorityUpdate = true;
		break;
	}
	case Recording::State::PROCESSING:
	{
		if (recording->savedFrames >= recording->frameCount()) {
			recording->state = Recording::State::FINISHED;
			break;
		}

		Frame frame = recording->getFrame(recording->savedFrames);
		frameCart->x = frame.x;
		frameCart->y = frame.y;
		frameCart->theta = frame.theta;
		frameCart->phi = frame.phi;
		frameDrawingDevice->setCamera(frame.camerax, frame.cameray, frame.zoom);
		frameDrawingDevice->beginDraw();
		paintScenery(frameDrawingDevice);
		frameCart->paint(frameDrawingDevice);
		frameDrawingDevice->endDraw();
		recording->savedFrames++;
		frameDrawingDevice->saveToFile(
			recording->getFolderName() + "\\frame" +
			std::to_string(recording->savedFrames) + ".png");
		break;
	}
	case Recording::State::FINISHED:
		frozen = false;
		priorityUpdate = false;
		delete frameCart;
		frameCart = nullptr;
		delete frameDrawingDevice;
		frameDrawingDevice = nullptr;
		delete recording;
		recording = nullptr;
		break;
	default:
		break;
	}
}

void Simulator::paint(DrawingDevice* drawingDevice)
{
	if (drawingDevice == nullptr)
		return;

	/* Drawing device width and height. */
	double width = drawingDevice->getWidth();
	double height = drawingDevice->getHeight();
	
	if (updateCamera) {
		drawingDevice->setCamera(cameraX, cameraY, cameraZoom);
		updateCamera = false;
	}

	/* Remember the last camera settings. */
	cameraX = drawingDevice->getCameraX();
	cameraY = drawingDevice->getCameraY();
	cameraZoom = drawingDevice->getCameraZoom();

	/* Draw background and floor. */
	paintScenery(drawingDevice);

	/* Draw cart */
	cart.paint(drawingDevice);

	/* Draw text */
	if (showInfo) {
		std::stringstream stream;
		stream << std::fixed;
		stream << std::setprecision(1);
		stream << "CPU: " << CPUUsage::getUsage() << "%";
		drawingDevice->screenText(stream.str(), 10, 10);
		stream.str(std::string());
		stream << "Action frequency: " << Engine::simulatorParameters.actionFrequency << " Hz";
		drawingDevice->screenText(stream.str(), 10, 30);
		stream.str(std::string());
		stream << "Simulation speed: " << Engine::simulatorParameters.simulationSpeed << "x";
		drawingDevice->screenText(stream.str(), 10, 50);
		stream.str(std::string());
		stream << "Manual force: " << Engine::simulatorParameters.manualForce << " N";
		drawingDevice->screenText(stream.str(), 10, 70);
		stream.str(std::string());
		stream << "Mass: " << Engine::simulatorParameters.cart.mass << " Kg / ";
		stream << Engine::simulatorParameters.pole.mass << " Kg";
		drawingDevice->screenText(stream.str(), 10, 90);
		stream.str(std::string());
		stream << "Damping: " << Engine::simulatorParameters.cart.damping;
		stream << " / " << Engine::simulatorParameters.pole.damping;
		drawingDevice->screenText(stream.str(), 10, 110);
		stream.str(std::string());
		if (Engine::isLoaded()) {
			if (Engine::simulatorParameters.engineName == nullptr || *Engine::simulatorParameters.engineName == 0)
				stream << "Engine loaded";
			else
				stream << "Engine: " << Engine::simulatorParameters.engineName;
		}
		else
			stream << "No engine";
		drawingDevice->screenText(stream.str(), 10, 130);
		drawingDevice->screenText("Press F1 for help", 10, height - 25);
	}

	/* Draw recorder rectangle. */
	if (showCameraFrame || recording != nullptr) {
		drawingDevice->animateObjects(Engine::simulatorParameters.actionFrequency);
		double hbar = (drawingDevice->getWidth() / 2) - 640;
		double vbar = (drawingDevice->getHeight() / 2) - 360;
		drawingDevice->screenRectangleEmpty(hbar, vbar, hbar + 1280, vbar + 720, 2.0,
			drawingDevice->brushText, drawingDevice->cameraStrokeStyle);
	}

	/* Draw log or help. */
	const char* logText = nullptr;
	if (showHelp)
		logText = Simulator::helpText;
	else if (showLog) {
		logText = log.c_str();
	}

	if (logText != nullptr) {
		/* Draw the log rectangle. */
		drawingDevice->screenRectangle(2 * width / 3, 0, width, height, drawingDevice->brushLog);

		/* Count the lines. */
		const char* pc = logText;
		int lines = (*pc != 0 ? 1 : 0);
		bool lastIsNewLine = false;
		while (*pc != 0) {
			if (*pc == '\n') {
				lines++;
				lastIsNewLine = true;
			}
			else {
				lastIsNewLine = false;
			}
			pc++;
		}
		if (lastIsNewLine) lines--;

		/* How many lines are visible? */
		double logWidth = width / 3;
		double logHeight = height;
		int visibleLines = static_cast<int>(std::floor(logHeight / 18));
		if (visibleLines > lines) visibleLines = lines;

		/* Skip invisible lines. */
		std::stringstream stream(logText);
		std::string line;
		for (int i = 0; i < lines - visibleLines; i++)
			std::getline(stream, line, '\n');

		/* Draw visible text. */
		double y = 0;
		while (std::getline(stream, line)) {
			drawingDevice->screenText(line, 2 * width / 3 + 2, y, logWidth - 4);
			y += 18;
		}
	}

	/* Display recording information. */
	if (recording != nullptr) {
		switch (recording->state) {
		case Recording::State::RECORDING:
		{
			int tempo = static_cast<int>(recording->time * 2);
			if (tempo % 2 == 0) {
				drawingDevice->screenTextMsg(
					"Recording",
					drawingDevice->getHeight() / 2 - 15,
					drawingDevice->brushTextRed
				);
			}
			break;
		}
		case Recording::State::PROCESSING:
		{
			std::string msg = "Processing frames [" + recording->getRecordingName() + "] ... " +
				std::to_string(recording->savedFrames + 1) + "/" +
				std::to_string(recording->frameCount());
			drawingDevice->screenTextMsg(msg, drawingDevice->getHeight() / 2 - 15);
			break;
		}
		default:
			break;
		}
	}
}

void Simulator::paintScenery(DrawingDevice* drawingDevice)
{
	/* Draw background. */
	drawingDevice->fillBackground();

	/* Draw markers. */
	if (Engine::simulatorParameters.markers != nullptr) {
		const Engine::Marker* pMarker = Engine::simulatorParameters.markers;
		while (pMarker->width > 0) {
			drawingDevice->stripe(
				pMarker->x,
				pMarker->width,
				pMarker->red,
				pMarker->green,
				pMarker->blue
			);
			pMarker++;
		}
	}

	/* Draw floor. */
	drawingDevice->polygonBezier(
		&floor[0],
		static_cast<int>(floor.size()),
		drawingDevice->brushFloor
	);
	drawingDevice->ground(-100, 100, -10, drawingDevice->brushFloor);
}

void Simulator::computeFloor()
{
	floor.clear();
	floor.push_back(DrawingDevice::Bezier(DrawingDevice::Point(), DrawingDevice::Point(-100, -11)));
	floor.push_back(DrawingDevice::Bezier(DrawingDevice::Point(-100, 0), DrawingDevice::Point(-100, 0)));
	
	double min = -100, max = 100;
	if (Engine::simulatorParameters.craters != nullptr) {
		const Engine::Crater* pCrater = Engine::simulatorParameters.craters;
		while (pCrater->width > 0) {
			double left = pCrater->x - pCrater->width / 2;
			double right = pCrater->x + pCrater->width / 2;
			bool valid =
				left >= min &&
				right <= max &&
				pCrater->width >= 6 * std::abs(pCrater->depth) &&
				std::abs(pCrater->depth) < 10;
			if (valid) {
				floor.push_back(
					DrawingDevice::Bezier(
						DrawingDevice::Point(left, 0),
						DrawingDevice::Point(left, 0)
					)
				);
				floor.push_back(
					DrawingDevice::Bezier(
						DrawingDevice::Point(left + 0.1 * pCrater->width, 0),
						DrawingDevice::Point(left + 0.25 * pCrater->width, -pCrater->depth / 2)
					)
				);
				floor.push_back(
					DrawingDevice::Bezier(
						DrawingDevice::Point(left + 0.4 * pCrater->width, -pCrater->depth),
						DrawingDevice::Point(left + 0.5 * pCrater->width, -pCrater->depth)
					)
				);
				floor.push_back(
					DrawingDevice::Bezier(
						DrawingDevice::Point(right - 0.4 * pCrater->width, -pCrater->depth),
						DrawingDevice::Point(right - 0.25 * pCrater->width, -pCrater->depth / 2)
					)
				);
				floor.push_back(
					DrawingDevice::Bezier(
						DrawingDevice::Point(right - 0.1 * pCrater->width, 0),
						DrawingDevice::Point(right, 0)
					)
				);
				min = right;
			}
			pCrater++;
		}
	}
	
	floor.push_back(DrawingDevice::Bezier(DrawingDevice::Point(100, 0), DrawingDevice::Point(100, 0)));
	floor.push_back(DrawingDevice::Bezier(DrawingDevice::Point(100, -11), DrawingDevice::Point(100, -11)));
}

void Simulator::alignCartWithFloor()
{
	double ycorrection = 0;
	cart.phi = computeCartAngle(cart.x, cart.getWheelDistance() / 2, 0.01, ycorrection);
	cart.y = getFloorHeight(cart.x) + ycorrection;
}

double Simulator::computeCartAngle(double x, double r, double epsilon, double& ycorrection)
{
	/* Central point on the floor. */
	double x0 = x;
	double y0 = getFloorHeight(x0);

	/* Find front point. */
	double x1 = 0;
	double y1 = 0;
	double min = x0;
	double max = x0 + r;
	double error = epsilon + 1;
	while (error > epsilon) {
		x1 = (min + max) / 2;
		y1 = getFloorHeight(x1);
		double xr = x1 - x0;
		double yr = y1 - y0;
		double dist = sqrt(xr * xr + yr * yr);
		error = fabs(dist - r);
		if (dist < r) min = x1;
		else if (dist > r) max = x1;
	}

	/* Find rear point. */
	double x2 = 0;
	double y2 = 0;
	min = x0;
	max = x0 - r;
	error = epsilon + 1;
	while (error > epsilon) {
		x2 = (min + max) / 2;
		y2 = getFloorHeight(x2);
		double xr = x2 - x0;
		double yr = y2 - y0;
		double dist = sqrt(xr * xr + yr * yr);
		error = fabs(dist - r);
		if (dist < r) min = x2;
		else if (dist > r) max = x2;
	}

	/* Compute the angle. */
	double angle = atan2(y1 - y2, x1 - x2);

	/* Compute correction of y, so the wheels touch the floor. */
	ycorrection = (y1 + y2) / 2 - y0;

	/* Return the angle. */
	return angle;
}

double Simulator::getFloorHeight(double x)
{
	double y = 0;
	for (size_t i = 1; i < floor.size() - 1; i++) {
		int result = computeBezierY(floor[i - 1].end, floor[i], x, y);
		if (result == 0) return y;
	}
	return 0;
}

int Simulator::computeBezierY(DrawingDevice::Point& point, DrawingDevice::Bezier& bezier, double x, double&y)
{
	double a = point.x - 2 * bezier.control.x + bezier.end.x;
	double b = 2 * bezier.control.x - 2 * point.x;
	double c = point.x - x;

	double d = b * b - 4 * a * c;
	if (d < 0) return -1;
	d = sqrt(d);

	double t1 = (-1 * b + d) / (2 * a);
	double t2 = (-1 * b - d) / (2 * a);

	int valid1 = (t1 >= 0 && t1 <= 1);
	int valid2 = (t2 >= 0 && t2 <= 1);

	if (!valid1 && !valid2) return -1;
	double t = (valid1 ? t1 : t2);

	double u = 1 - t;
	y = u * u * point.y + 2 * u * t * bezier.control.y + t * t * bezier.end.y;

	return 0;
}

void Simulator::updateLog()
{
	if (Engine::simulatorParameters.pLogBuffer != nullptr && *Engine::simulatorParameters.pLogBuffer != 0)
		log.append(Engine::simulatorParameters.pLogBuffer);
}

void Simulator::saveLog(const char* filename)
{
	if (filename == nullptr)
		return;

	std::ofstream file(filename, std::ios_base::out | std::ios_base::app);
	if (file.is_open()) {
		file << log.c_str();
		file.close();
	}
}