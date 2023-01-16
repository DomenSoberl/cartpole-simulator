#include <string>
#include "drawingdevice.h"
#include "engine.h"

const double DrawingDevice::ppm = 100;

DrawingDevice::DrawingDevice(HWND hwnd) :
	hwnd(hwnd),
	d2dFactory(nullptr),
	hwndRenderer(nullptr),
	bmpRenderer(nullptr),
	renderer(nullptr),
	writeFactory(nullptr),
	textFormat(nullptr),
	textFormatMsg(nullptr),
	imagingFactory(nullptr),
	bitmap(nullptr),
	brushBackground(nullptr),
	brushPointer(nullptr),
	brushFloor(nullptr),
	brushCart(nullptr),
	brushTire(nullptr),
	brushWheel(nullptr),
	brushPole(nullptr),
	brushPoleBall(nullptr),
	brushText(nullptr),
	brushTextRed(nullptr),
	brushLog(nullptr),
	brushCustom(nullptr),
	cameraStrokeStyle(nullptr),
	width(0),
	height(0),
	camerax(0),
	cameray(0),
	zoom(1)
{
	if (createFactories()) {
		resize();
		resetCamera();
	}
	Engine::simulatorParameters.camera.y = -0.5 * s2wy(height);
}

DrawingDevice::DrawingDevice(int width, int height) :
	hwnd(nullptr),
	d2dFactory(nullptr),
	hwndRenderer(nullptr),
	bmpRenderer(nullptr),
	renderer(nullptr),
	writeFactory(nullptr),
	textFormat(nullptr),
	textFormatMsg(nullptr),
	imagingFactory(nullptr),
	bitmap(nullptr),
	brushBackground(nullptr),
	brushPointer(nullptr),
	brushFloor(nullptr),
	brushCart(nullptr),
	brushTire(nullptr),
	brushWheel(nullptr),
	brushPole(nullptr),
	brushPoleBall(nullptr),
	brushText(nullptr),
	brushTextRed(nullptr),
	brushLog(nullptr),
	brushCustom(nullptr),
	cameraStrokeStyle(nullptr),
	width(width),
	height(height),
	camerax(0),
	cameray(0),
	zoom(1)
{
	if (createFactories()) {
		createAssets();
		resetCamera();
	}	
}

DrawingDevice::~DrawingDevice()
{
	for (ID2D1SolidColorBrush* brush : brushes)
		brush->Release();
	brushes.clear();

	if (cameraStrokeStyle != nullptr)
		cameraStrokeStyle->Release();

	if (bitmap != nullptr)
		bitmap->Release();

	if (textFormat != nullptr)
		textFormat->Release();

	if (writeFactory != nullptr)
		writeFactory->Release();

	if (hwndRenderer != nullptr)
		hwndRenderer->Release();

	if (bmpRenderer != nullptr)
		bmpRenderer->Release();

	if (imagingFactory != nullptr)
		imagingFactory->Release();

	if (d2dFactory != nullptr)
		d2dFactory->Release();
}

bool DrawingDevice::createFactories()
{
	HRESULT result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);
	if (result != S_OK || d2dFactory == nullptr)
		return false;

	result = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&writeFactory)
	);
	if (result != S_OK || writeFactory == nullptr)
		return false;

	if (hwnd == nullptr) {
		result = CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory,
			reinterpret_cast<void**>(&imagingFactory)
		);
		if (result != S_OK || imagingFactory == nullptr)
			return false;

		result = imagingFactory->CreateBitmap(
			static_cast<int>(width),
			static_cast<int>(height),
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapCacheOnDemand,
			&bitmap
		);
		if (result != S_OK || bitmap == nullptr)
			return false;

		D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = {
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			{DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED},
			0.0f, 0.0f,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		};

		result = d2dFactory->CreateWicBitmapRenderTarget(bitmap, renderTargetProperties, &bmpRenderer);
		if (result != S_OK || bmpRenderer == nullptr)
			return false;

		renderer = bmpRenderer;
	}

	return true;
}

void DrawingDevice::createAssets()
{
	/* Assets that need to be created only once. */
	if (textFormat == nullptr) {
		HRESULT result = writeFactory->CreateTextFormat(
			L"Consolas",
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			14,
			L"",
			&textFormat
		);
		if (result != S_OK || textFormat == nullptr)
			return;

		textFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	}

	if (textFormatMsg == nullptr) {
		HRESULT result = writeFactory->CreateTextFormat(
			L"Consolas",
			nullptr,
			DWRITE_FONT_WEIGHT_MEDIUM,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			24,
			L"",
			&textFormatMsg
		);
		if (result != S_OK || textFormatMsg == nullptr)
			return;

		textFormatMsg->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		textFormatMsg->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	}

	if (cameraStrokeStyle == nullptr) {
		D2D1_STROKE_STYLE_PROPERTIES properties = {
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_LINE_JOIN_MITER,
			1.0f,
			D2D1_DASH_STYLE_DASH_DOT,
			0.0f
		};
		HRESULT result = d2dFactory->CreateStrokeStyle(properties, nullptr, 0, &cameraStrokeStyle);
		if (result != S_OK || textFormatMsg == nullptr)
			return;
	}

	/* Assets that need to be created every time the screen is resized. */
	for (ID2D1SolidColorBrush* brush : brushes)
		brush->Release();
	brushes.clear();

	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00408080, 1.0f), &brushBackground);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00ffffff, 1.0f), &brushPointer);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00111111, 1.0f), &brushFloor);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00800000, 1.0f), &brushCart);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00000000, 1.0f), &brushTire);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00402020, 1.0f), &brushWheel);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00402020, 1.0f), &brushPole);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00000000, 1.0f), &brushPoleBall);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00ffffff, 1.0f), &brushText);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00ff0000, 1.0f), &brushTextRed);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00191970, 0.3f), &brushLog);
	renderer->CreateSolidColorBrush(D2D1::ColorF(0x00000000, 1.0f), &brushCustom);

	if (brushBackground != nullptr) brushes.push_back(brushBackground);
	if (brushPointer != nullptr) brushes.push_back(brushPointer);
	if (brushFloor != nullptr) brushes.push_back(brushFloor);
	if (brushCart != nullptr) brushes.push_back(brushCart);
	if (brushTire != nullptr) brushes.push_back(brushTire);
	if (brushWheel != nullptr) brushes.push_back(brushWheel);
	if (brushPole != nullptr) brushes.push_back(brushPole);
	if (brushPoleBall != nullptr) brushes.push_back(brushPoleBall);
	if (brushText != nullptr) brushes.push_back(brushText);
	if (brushText != nullptr) brushes.push_back(brushTextRed);
	if (brushLog != nullptr) brushes.push_back(brushLog);
	if (brushCustom != nullptr) brushes.push_back(brushCustom);
}

void DrawingDevice::resize()
{
	if (hwnd == nullptr || d2dFactory == nullptr)
		return;

	if (hwndRenderer != nullptr) {
		hwndRenderer->Release();
		hwndRenderer = nullptr;
	}

	RECT rc;
	GetClientRect(hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	d2dFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, size, D2D1_PRESENT_OPTIONS_NONE),
		&hwndRenderer
	);

	if (hwndRenderer != nullptr) {
		D2D1_SIZE_F rendererSize = hwndRenderer->GetSize();
		width = static_cast<double>(rendererSize.width);
		height = static_cast<double>(rendererSize.height);
		renderer = hwndRenderer;
	}

	createAssets();
}

void DrawingDevice::moveCamera(double dx, double dy)
{
	camerax += dx;
	cameray += dy;
}

void DrawingDevice::zoomIn(double factor)
{
	zoom *= factor;
}

void DrawingDevice::setCamera(double x, double y, double zoom)
{
	this->camerax = x;
	this->cameray = y;
	this->zoom = zoom;
}

void DrawingDevice::resetCamera()
{
	camerax = Engine::simulatorParameters.camera.x;
	cameray = Engine::simulatorParameters.camera.y;
	zoom = Engine::simulatorParameters.camera.zoom;
}

void DrawingDevice::beginDraw()
{
	if (renderer == nullptr)
		return;

	renderer->BeginDraw();
}

void DrawingDevice::endDraw()
{
	if (renderer == nullptr)
		return;

	renderer->EndDraw();
}

void DrawingDevice::fillBackground()
{
	if (renderer == nullptr)
		return;

	renderer->Clear(brushBackground->GetColor());
}

void DrawingDevice::circle(Point& center, double radius, ID2D1SolidColorBrush* color)
{
	if (renderer == nullptr)
		return;

	float r = static_cast<float>(w2sm(radius));
	D2D1_ELLIPSE ellipse = D2D1::Ellipse(
		D2D1::Point2F(
			static_cast<float>(w2sx(center.x)),
			static_cast<float>(w2sy(center.y))
		),
		r, r
	);
	renderer->FillEllipse(ellipse, color);
}

void DrawingDevice::polygon(Point* points, int n, ID2D1SolidColorBrush* color)
{
	if (renderer == nullptr)
		return;

	ID2D1PathGeometry* path = nullptr;
	HRESULT hr = d2dFactory->CreatePathGeometry(&path);
	if (!SUCCEEDED(hr)) return;

	ID2D1GeometrySink* sink = nullptr;
	hr = path->Open(&sink);
	if (!SUCCEEDED(hr)) return;

	sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	
	sink->BeginFigure(
		D2D1::Point2F(
			static_cast<float>(w2sx(points[0].x)),
			static_cast<float>(w2sy(points[0].y))
		),
		D2D1_FIGURE_BEGIN_FILLED
	);
	for (int i = 1; i < n; i++) {
		sink->AddLine(
			D2D1::Point2F(
				static_cast<float>(w2sx(points[i].x)),
				static_cast<float>(w2sy(points[i].y))
			)
		);
	}
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();

	renderer->FillGeometry(path, color);
	sink->Release();
	path->Release();
}

void DrawingDevice::polygonBezier(Bezier* segments, int n, ID2D1SolidColorBrush* color)
{
	if (renderer == nullptr)
		return;

	ID2D1PathGeometry* path = nullptr;
	HRESULT hr = d2dFactory->CreatePathGeometry(&path);
	if (!SUCCEEDED(hr)) return;

	ID2D1GeometrySink* sink = nullptr;
	hr = path->Open(&sink);
	if (!SUCCEEDED(hr)) return;

	sink->SetFillMode(D2D1_FILL_MODE_WINDING);

	sink->BeginFigure(
		D2D1::Point2F(
			static_cast<float>(w2sx(segments[0].end.x)),
			static_cast<float>(w2sy(segments[0].end.y))
		),
		D2D1_FIGURE_BEGIN_FILLED
	);
	for (int i = 1; i < n; i++) {
		sink->AddQuadraticBezier(
			D2D1::QuadraticBezierSegment(
				D2D1::Point2F(
					static_cast<float>(w2sx(segments[i].control.x)),
					static_cast<float>(w2sy(segments[i].control.y))
				),
				D2D1::Point2F(
					static_cast<float>(w2sx(segments[i].end.x)),
					static_cast<float>(w2sy(segments[i].end.y))
				)
			)
		);
	}
	sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	sink->Close();

	renderer->FillGeometry(path, color);
	sink->Release();
	path->Release();
}

void DrawingDevice::ground(double left, double right, double top, ID2D1SolidColorBrush* color)
{
	if (renderer == nullptr)
		return;

	D2D1_RECT_F rect = D2D1::RectF(
		static_cast<float>(w2sx(left)),
		static_cast<float>(w2sy(top)),
		static_cast<float>(w2sx(right)),
		static_cast<float>(height)
	);
	renderer->FillRectangle(rect, color);
}

void DrawingDevice::stripe(double x, double width, unsigned char red, unsigned char green, unsigned char blue)
{
	if (renderer == nullptr)
		return;

	D2D1_RECT_F rect = D2D1::RectF(
		static_cast<float>(w2sx(x - width / 2)),
		static_cast<float>(0),
		static_cast<float>(w2sx(x + width / 2)),
		static_cast<float>(height)
	);
	D2D1::ColorF color(
		static_cast<float>(red) / 255.0f,
		static_cast<float>(green) / 255.0f,
		static_cast<float>(blue) / 255.0f,
		1.0f
	);
	brushCustom->SetColor(&color);
	renderer->FillRectangle(rect, brushCustom);
}

void DrawingDevice::screenRectangle(double x1, double y1, double x2, double y2, ID2D1SolidColorBrush* color)
{
	if (renderer == nullptr)
		return;

	D2D1_RECT_F rect = D2D1::RectF(
		static_cast<float>(x1),
		static_cast<float>(y1),
		static_cast<float>(x2),
		static_cast<float>(y2)
	);
	renderer->FillRectangle(rect, color);
}

void DrawingDevice::screenRectangleEmpty(
	double x1,
	double y1,
	double x2,
	double y2,
	double width,
	ID2D1SolidColorBrush* color,
	ID2D1StrokeStyle* style
) {
	if (renderer == nullptr)
		return;

	D2D1_RECT_F rect = D2D1::RectF(
		static_cast<float>(x1),
		static_cast<float>(y1),
		static_cast<float>(x2),
		static_cast<float>(y2)
	);

	renderer->DrawRectangle(rect, color, static_cast<float>(width), style);
}

void DrawingDevice::screenText(std::string str, double x, double y, double lineWidth, ID2D1SolidColorBrush* color)
{
	if (renderer == nullptr || textFormat == nullptr)
		return;

	std::wstring wstr = std::wstring(str.begin(), str.end());
	D2D1_RECT_F rect = D2D1::RectF(
		static_cast<float>(x),
		static_cast<float>(y),
		static_cast<float>(x + (lineWidth > 0 ? lineWidth : width)),
		static_cast<float>(y + 18)
	);
	renderer->DrawText(
		wstr.c_str(),
		static_cast<int>(wstr.length()),
		textFormat,
		rect,
		(color != nullptr ? color: brushText),
		D2D1_DRAW_TEXT_OPTIONS_NO_SNAP | D2D1_DRAW_TEXT_OPTIONS_CLIP
	);
}

void DrawingDevice::screenTextMsg(std::string str, double y, ID2D1SolidColorBrush* color)
{
	if (renderer == nullptr || textFormat == nullptr)
		return;

	std::wstring wstr = std::wstring(str.begin(), str.end());
	D2D1_RECT_F rect = D2D1::RectF(
		static_cast<float>(0),
		static_cast<float>(y),
		static_cast<float>(width),
		static_cast<float>(y + 30)
	);
	renderer->DrawText(
		wstr.c_str(),
		static_cast<int>(wstr.length()),
		textFormatMsg,
		rect,
		(color != nullptr ? color : brushText),
		D2D1_DRAW_TEXT_OPTIONS_NONE
	);
}

bool DrawingDevice::saveToFile(std::string filename)
{
	IWICStream* stream = nullptr;
	HRESULT result = imagingFactory->CreateStream(&stream);
	if (result != S_OK || stream == nullptr)
		return false;

	std::wstring wfilename = std::wstring(filename.begin(), filename.end());
	result = stream->InitializeFromFilename(wfilename.c_str(), GENERIC_WRITE);
	if (result != S_OK) {
		stream->Release();
		return false;
	}

	IWICBitmapEncoder* encoder = nullptr;
	result = imagingFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder);
	if (result != S_OK || encoder == nullptr) {
		stream->Release();
		return false;
	}

	result = encoder->Initialize(stream, WICBitmapEncoderNoCache);

	IWICBitmapFrameEncode* frame = nullptr;
	if (result == S_OK)
		result = encoder->CreateNewFrame(&frame,nullptr);

	UINT bmpWidth = 0, bmpHeight = 0;
	WICPixelFormatGUID pixelFormat = GUID_WICPixelFormatDontCare;
	bitmap->GetSize(&bmpWidth, &bmpHeight);

	if (result == S_OK)
		result = frame->Initialize(nullptr);

	if (result == S_OK)
		result = frame->SetSize(bmpWidth, bmpHeight);
	if (result == S_OK)
		result = frame->SetPixelFormat(&pixelFormat);
	if (result == S_OK)
		result = frame->WriteSource(bitmap, nullptr);
	if (result == S_OK)
		result = frame->Commit();
	if (result == S_OK)
		result = encoder->Commit();

	frame->Release();
	encoder->Release();
	stream->Release();

	return (result == S_OK);
}

void DrawingDevice::animateObjects(double frequency)
{
	/* Animate camera stroke style. */
	float dashOffset = 0.0f;
	if (cameraStrokeStyle != nullptr) {
		dashOffset = cameraStrokeStyle->GetDashOffset();
		cameraStrokeStyle->Release();
		cameraStrokeStyle = nullptr;
	}
	D2D1_STROKE_STYLE_PROPERTIES properties = {
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_CAP_STYLE_FLAT,
			D2D1_LINE_JOIN_MITER,
			1.0f,
			D2D1_DASH_STYLE_DASH_DOT,
			dashOffset + (20.0f / static_cast<float>(frequency))
	};
	d2dFactory->CreateStrokeStyle(properties, nullptr, 0, &cameraStrokeStyle);
}