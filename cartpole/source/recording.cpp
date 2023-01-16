#include <Windows.h>
#include "recording.h"
#include "drawingdevice.h"

Frame::Frame() :
	F(0),
	x(0),
	y(0),
	theta(0),
	phi(0),
	dx(0),
	ddx(0),
	dtheta(0),
	ddtheta(0),
	camerax(0),
	cameray(0),
	zoom(1)
{
}

Frame::Frame(
	double F,
	double x,
	double y,
	double theta,
	double phi,
	double dx,
	double ddx,
	double dtheta,
	double ddtheta,
	double camerax,
	double cameray,
	double zoom
) :
	F(F),
	x(x),
	y(y),
	theta(theta),
	phi(phi),
	dx(dx),
	ddx(ddx),
	dtheta(dtheta),
	ddtheta(ddtheta),
	camerax(camerax),
	cameray(cameray),
	zoom(zoom)
{
}

Frame::~Frame()
{
}

void Frame::saveData(HANDLE file, int i, double time)
{
	std::string data = std::to_string(i) + ";";
	data += std::to_string(time) + ";";
	data += std::to_string(F) + ";";
	data += std::to_string(x) + ";";
	data += std::to_string(y) + ";";
	data += std::to_string(theta) + ";";
	data += std::to_string(phi) + ";";
	data += std::to_string(dx) + ";";
	data += std::to_string(ddx) + ";";
	data += std::to_string(dtheta) + ";";
	data += std::to_string(ddtheta) + "\n";
	DWORD bytesWritten;
	WriteFile(file, data.c_str(), static_cast<DWORD>(data.length()), &bytesWritten, nullptr);
}

Recording::Recording(double fps) :
	state(State::RECORDING),
	time(0),
	savedFrames(0),
	fps(fps),
	folderName(""),
	recordingName("")
{
}

Recording::~Recording()
{
}

Frame Recording::getFrame(int i)
{
	if (i >= static_cast<int>(frames.size()))
		return Frame();
	return frames[i];
}

void Recording::snap(
	double F,
	double x,
	double y,
	double theta,
	double phi,
	double dx,
	double ddx,
	double dtheta,
	double ddtheta,
	double camerax,
	double cameray,
	double zoom
) {
	frames.push_back(
		Frame(
			F,
			x,
			y,
			theta,
			phi,
			dx,
			ddx,
			dtheta,
			ddtheta,
			camerax,
			cameray,
			zoom
		)
	);
}

bool Recording::createFolder()
{
	int i = 1;
	bool error = false;
	recordingName = "recording 1";
	folderName = "recording1";
	while (!CreateDirectoryA(folderName.c_str(), nullptr) && !error) {
		i++;
		recordingName = "recording " + std::to_string(i);
		folderName = "recording" + std::to_string(i);
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			error = true;
	}

	return !error;
}

void Recording::saveFramesData()
{
	if (folderName.empty()) return;
	std::string fileName = folderName + "\\" + "frames.csv";
	HANDLE file = CreateFileA(fileName.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	static char header[] = "frame;time;F;x;y;theta;phi;x';x'';theta';theta''\n";
	DWORD bytesWritten;
	WriteFile(file, header, sizeof(header) - 1, &bytesWritten, nullptr);
	double time = 0;
	double dt = 1.0 / fps;
	for (int i = 0; i < static_cast<int>(frames.size()); i++) {
		frames[i].saveData(file, i + 1, time);
		time += dt;
	}
	CloseHandle(file);
}