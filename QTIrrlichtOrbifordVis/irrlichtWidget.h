#pragma once

#include <QWidget>
#include <QResizeEvent>
#include <irrlicht.h>
#include "driverChoice.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class MyEventReceiver;

class irrlichtWidget : public QWidget
{
	Q_OBJECT

public:
	irrlichtWidget(QWidget *parent = NULL);
	~irrlichtWidget();

	// Returns a pointer to the Irrlicht Device
	IrrlichtDevice* getIrrlichtDevice();

	// Create the Irrlicht device and connect the signals and slots
	void init();
	void initalizeScene();
	void updateToolBox();
	void setActiveCamera(scene::ICameraSceneNode* newActive);
	void setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin);
	void updateScaleInfo(scene::ISceneNode* model);
	void showAboutText();
	void loadModel(const c8* fn);
	void createToolBox();
	void onKillFocus();
	bool hasModalDialog();

	core::stringc StartUpModelFile;
	core::stringw MessageText;
	core::stringw Caption;
	scene::ISceneNode* Model = 0;
	scene::ISceneNode* SkyBox = 0;
	bool Octree = false;
	bool UseLight = false;

	ICameraSceneNode* Camera[2] = { 0, 0 };
	bool MouseInWidget;


signals:
	// Signal that its time to update the frame
	void updateIrrlichtQuery(IrrlichtDevice* device);

public slots:
	// Function called in response to updateIrrlichtQuery. Renders the scene in the widget
	void updateIrrlicht(IrrlichtDevice* device);

protected:
	virtual void paintEvent(QPaintEvent* event);
	virtual void timerEvent(QTimerEvent* event);
	virtual void resizeEvent(QResizeEvent* event);

	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

	enum MOUSEBUTTONSTATE {
		NOMOUSEBUTTON,
		MOUSELEFT,
		MOUSERIGHT,
		MOUSEMIDDLE,
	} m_mousebutton_state;

	void sendKeyEventToIrrlicht(QKeyEvent* event, bool pressedDown);
	void sendMouseEventToIrrlicht(QMouseEvent* event, bool pressedDown);

	QPoint lastLoc;

	IrrlichtDevice *device;
	IVideoDriver *driver;
	// We keep the camera inside this widget so we can resize the window dynamically
	//ICameraSceneNode* camera;
	IGUIEnvironment* env;
	IGUIStaticText* fpstext;
	IGUIStaticText* postext;
	MyEventReceiver *receiver;
	ISceneManager *smgr;
};
