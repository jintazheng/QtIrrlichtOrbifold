#pragma once

#include <irrlicht.h>
#include "driverChoice.h"
#include <iostream>
#include <memory>

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

using namespace irr;
using namespace gui;

class QTIrrlichtOrbifordVis;

class VisView :public QThread
{
	Q_OBJECT

public:
	VisView(QTIrrlichtOrbifordVis* qt, QObject *parent = 0);
	~VisView();

	/*
	Some global variables used later on
	*/
	IrrlichtDevice *Device = 0;
	core::stringc StartUpModelFile;
	core::stringw MessageText;
	core::stringw Caption;
	scene::ISceneNode* Model = 0;
	scene::ISceneNode* SkyBox = 0;
	bool Octree = false;
	bool UseLight = false;

	scene::ICameraSceneNode* Camera[2] = { 0, 0 };
	scene::ISceneManager* smgr;
	video::IVideoDriver* driver;
	IGUIEnvironment* env;
	IGUIStaticText* fpstext;
	IGUIStaticText* postext;

	void VisView::setActiveCamera(scene::ICameraSceneNode* newActive);
	void VisView::setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin);
	void VisView::updateScaleInfo(scene::ISceneNode* model);
	void VisView::showAboutText();
	void VisView::loadModel(const c8* fn);
	void VisView::createToolBox();
	void updateToolBox();
	void VisView::onKillFocus();
	bool VisView::hasModalDialog();

	void setQTDialog(QTIrrlichtOrbifordVis* qt);


	void startVisView();

protected:
	void run() override;
	void stop();

private:
	QMutex mutex;
	QWaitCondition condition;
	bool restart;
	bool abort;

	std::unique_ptr<QTIrrlichtOrbifordVis> m_qt;
};

