#pragma once
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <string>
#include <vector>

class DrawingDevice
{
public:
	class Point {
	public:
		Point() : x(0), y(0) {}
		Point(double x, double y) : x(x), y(y) {}
		~Point() {}
		double x, y;
	};

	class Bezier {
	public:
		Bezier(
			DrawingDevice::Point control,
			DrawingDevice::Point end
		) : control(control),
			end(end) {}
		~Bezier() {}
		DrawingDevice::Point control;
		DrawingDevice::Point end;
	};

	DrawingDevice() = delete;
	DrawingDevice(HWND hwnd);
	DrawingDevice(int width, int height);
	~DrawingDevice();

	int isInitialized() const { return renderer != nullptr; } ;
	void resize();

	double getWidth() const { return width; }
	double getHeight() const { return height; }
	double getCameraX() const { return camerax; }
	double getCameraY() const { return cameray; }
	double getCameraZoom() const { return zoom; }

	void moveCamera(double dx, double dy);
	void zoomIn(double factor);
	void setCamera(double x, double y, double zoom);
	void resetCamera();

	inline double w2sx(double x) {
		return ((width / 2) + (x - camerax) * (zoom * ppm));
	}

	inline double w2sy(double y) {
		return ((height / 2) - (y - cameray) * (zoom * ppm));
	}

	inline void w2s(Point & point) {
		point.x = w2sx(point.x);
		point.y = w2sy(point.y);
	}

	inline double s2wx(double x) {
		return ((x - (width / 2)) / (zoom *ppm) + camerax);
	}

	inline double s2wy(double y) {
		return (((height / 2) - y) / (zoom *ppm) + cameray);
	}

	inline void s2w(Point& point) {
		point.x = s2wx(point.x);
		point.y = s2wy(point.y);
	}

	inline double w2sm(double m) {
		return (m * (zoom * ppm));
	}

	inline double s2wm(double m) {
		return (m / (zoom * ppm));
	}

	void beginDraw();
	void endDraw();
	void fillBackground();
	void circle(Point& center, double radius, ID2D1SolidColorBrush* color);
	void polygon(Point* points, int n, ID2D1SolidColorBrush* color);
	void polygonBezier(Bezier* segments, int n, ID2D1SolidColorBrush* color);
	void ground(double left, double right, double top, ID2D1SolidColorBrush* color);
	void stripe(double x, double width, unsigned char red, unsigned char green, unsigned char blue);
	void screenRectangle(double x1, double y1, double x2, double y2, ID2D1SolidColorBrush* color);
	void screenRectangleEmpty(double x1, double y1, double x2, double y2, double width, 
		ID2D1SolidColorBrush* color, ID2D1StrokeStyle* style);
	void screenText(std::string str, double x, double y, double lineWidth = 0,
		ID2D1SolidColorBrush* color = nullptr);
	void screenTextMsg(std::string str, double y, ID2D1SolidColorBrush* color = nullptr);
	bool saveToFile(std::string filename);
	void animateObjects(double frequency);

protected:
	HWND hwnd;
	ID2D1Factory* d2dFactory;
	ID2D1HwndRenderTarget* hwndRenderer;
	ID2D1RenderTarget* bmpRenderer;
	ID2D1RenderTarget* renderer;
	IDWriteFactory* writeFactory;
	IDWriteTextFormat* textFormat;
	IDWriteTextFormat* textFormatMsg;
	IWICImagingFactory* imagingFactory;
	IWICBitmap* bitmap;
	std::vector<ID2D1SolidColorBrush*> brushes;

	bool createFactories();
	void createAssets();
	
public:
	ID2D1SolidColorBrush* brushBackground;
	ID2D1SolidColorBrush* brushPointer;
	ID2D1SolidColorBrush* brushFloor;
	ID2D1SolidColorBrush* brushCart;
	ID2D1SolidColorBrush* brushTire;
	ID2D1SolidColorBrush* brushWheel;
	ID2D1SolidColorBrush* brushPole;
	ID2D1SolidColorBrush* brushPoleBall;
	ID2D1SolidColorBrush* brushText;
	ID2D1SolidColorBrush* brushTextRed;
	ID2D1SolidColorBrush* brushLog;
	ID2D1SolidColorBrush* brushCustom;
	ID2D1StrokeStyle* cameraStrokeStyle;

private:
	static const double ppm;

	double width;
	double height;
	double camerax;
	double cameray;
	double zoom;
};