#pragma once
#include <string>
#include <vector>

class Frame
{
public:
	Frame();
	Frame(
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
	);
	~Frame();

	double F;
	double x;
	double y;
	double theta;
	double phi;
	double dx;
	double ddx;
	double dtheta;
	double ddtheta;
	double camerax;
	double cameray;
	double zoom;

	void saveData(HANDLE file, int i, double time);
};

class Recording
{
public:
	enum class State {
		RECORDING,
		STOPPED,
		PROCESSING,
		FINISHED
	};

	Recording() = delete;
	Recording(double fps);
	~Recording();

	State state;
	double time;
	int savedFrames;

	int frameCount() const { return static_cast<int>(frames.size()); }
	Frame getFrame(int i);
	void snap(
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
	);
	bool createFolder();
	void saveFramesData();
	std::string getRecordingName() const { return recordingName; }
	std::string getFolderName() const { return folderName; }

protected:
	double fps;
	std::vector<Frame> frames;

private:
	std::string folderName;
	std::string recordingName;
};