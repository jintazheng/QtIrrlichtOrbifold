#include "VisView.h"
#include "VisEventListener.h"
#include "QTIrrlichtOrbifordVis.h"
// Values used to identify individual GUI elements


VisView::VisView(QTIrrlichtOrbifordVis* qt, QObject *parent)
{

	/*Initialization for QT*/
	setQTDialog(qt);
	qt->setVisView(this);

	restart = false;
	abort = false;


	/*
	Most of the hard work is done. We only need to create the Irrlicht Engine
	device and all the buttons, menus and toolbars. We start up the engine as
	usual, using createDevice(). To make our application catch events, we set our
	eventreceiver as parameter. As you can see, there is also a call to
	IrrlichtDevice::setResizeable(). This makes the render window resizeable, which
	is quite useful for a mesh viewer.
	*/

	// ask user for driver
	video::E_DRIVER_TYPE driverType = video::E_DRIVER_TYPE::EDT_OPENGL;
	if (driverType == video::EDT_COUNT)
		return;

	// create device and exit if creation failed
	//MyEventReceiver receiver(this);
	Device = createDevice(driverType, core::dimension2d<u32>(800, 600),
		16, false, false, false, false);

	if (Device == 0)
		return; // could not create selected driver.

	Device->setResizable(true);

	Device->setWindowCaption(L"Irrlicht Engine - Loading...");

	driver = Device->getVideoDriver();
	env = Device->getGUIEnvironment();
	smgr = Device->getSceneManager();
	smgr->getParameters()->setAttribute(scene::COLLADA_CREATE_SCENE_INSTANCES, true);

	driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

	smgr->addLightSceneNode(0, core::vector3df(200, 200, 200),
		video::SColorf(1.0f, 1.0f, 1.0f), 2000);
	smgr->setAmbientLight(video::SColorf(0.3f, 0.3f, 0.3f));
	// add our media directory as "search path"
	Device->getFileSystem()->addFileArchive("./../media/");

	/*
	The next step is to read the configuration file. It is stored in the xml
	format and looks a little bit like this:

	@verbatim
	<?xml version="1.0"?>
	<config>
	<startUpModel file="some filename" />
	<messageText caption="Irrlicht Engine Mesh Viewer">
	Hello!
	</messageText>
	</config>
	@endverbatim

	We need the data stored in there to be written into the global variables
	StartUpModelFile, MessageText and Caption. This is now done using the
	Irrlicht Engine integrated XML parser:
	*/

	// read configuration from xml file

	io::IXMLReader* xml = Device->getFileSystem()->createXMLReader(L"config.xml");

	while (xml && xml->read())
	{
		switch (xml->getNodeType())
		{
		case io::EXN_TEXT:
			// in this xml file, the only text which occurs is the
			// messageText
			MessageText = xml->getNodeData();
			break;
		case io::EXN_ELEMENT:
		{
			if (core::stringw("startUpModel") == xml->getNodeName())
				StartUpModelFile = xml->getAttributeValue(L"file");
			else
				if (core::stringw("messageText") == xml->getNodeName())
					Caption = xml->getAttributeValue(L"caption");
		}
		break;
		default:
			break;
		}
	}

	if (xml)
		xml->drop(); // don't forget to delete the xml reader

	//if (argc > 1)
	//	StartUpModelFile = argv[1];

	/*
	That wasn't difficult. Now we'll set a nicer font and create the Menu.
	It is possible to create submenus for every menu item. The call
	menu->addItem(L"File", -1, true, true); for example adds a new menu
	Item with the name "File" and the id -1. The following parameter says
	that the menu item should be enabled, and the last one says, that there
	should be a submenu. The submenu can now be accessed with
	menu->getSubMenu(0), because the "File" entry is the menu item with
	index 0.
	*/

	// set a nicer font

	IGUISkin* skin = env->getSkin();
	IGUIFont* font = env->getFont("fonthaettenschweiler.bmp");
	if (font)
		skin->setFont(font);

	// create menu
	gui::IGUIContextMenu* menu = env->addMenu();
	menu->addItem(L"File", -1, true, true);
	menu->addItem(L"View", -1, true, true);
	menu->addItem(L"Camera", -1, true, true);
	menu->addItem(L"Help", -1, true, true);

	gui::IGUIContextMenu* submenu;
	submenu = menu->getSubMenu(0);
	submenu->addItem(L"Open Model File & Texture...", GUI_ID_OPEN_MODEL);
	submenu->addItem(L"Set Model Archive...", GUI_ID_SET_MODEL_ARCHIVE);
	submenu->addItem(L"Load as Octree", GUI_ID_LOAD_AS_OCTREE);
	submenu->addSeparator();
	submenu->addItem(L"Quit", GUI_ID_QUIT);

	submenu = menu->getSubMenu(1);
	submenu->addItem(L"sky box visible", GUI_ID_SKY_BOX_VISIBLE, true, false, true);
	submenu->addItem(L"toggle model debug information", GUI_ID_TOGGLE_DEBUG_INFO, true, true);
	submenu->addItem(L"model material", -1, true, true);

	submenu = submenu->getSubMenu(1);
	submenu->addItem(L"Off", GUI_ID_DEBUG_OFF);
	submenu->addItem(L"Bounding Box", GUI_ID_DEBUG_BOUNDING_BOX);
	submenu->addItem(L"Normals", GUI_ID_DEBUG_NORMALS);
	submenu->addItem(L"Skeleton", GUI_ID_DEBUG_SKELETON);
	submenu->addItem(L"Wire overlay", GUI_ID_DEBUG_WIRE_OVERLAY);
	submenu->addItem(L"Half-Transparent", GUI_ID_DEBUG_HALF_TRANSPARENT);
	submenu->addItem(L"Buffers bounding boxes", GUI_ID_DEBUG_BUFFERS_BOUNDING_BOXES);
	submenu->addItem(L"All", GUI_ID_DEBUG_ALL);

	submenu = menu->getSubMenu(1)->getSubMenu(2);
	submenu->addItem(L"Solid", GUI_ID_MODEL_MATERIAL_SOLID);
	submenu->addItem(L"Transparent", GUI_ID_MODEL_MATERIAL_TRANSPARENT);
	submenu->addItem(L"Reflection", GUI_ID_MODEL_MATERIAL_REFLECTION);

	submenu = menu->getSubMenu(2);
	submenu->addItem(L"Maya Style", GUI_ID_CAMERA_MAYA);
	submenu->addItem(L"First Person", GUI_ID_CAMERA_FIRST_PERSON);

	submenu = menu->getSubMenu(3);
	submenu->addItem(L"About", GUI_ID_ABOUT);

	/*
	Below the menu we want a toolbar, onto which we can place colored
	buttons and important looking stuff like a senseless combobox.
	*/

	// create toolbar

	gui::IGUIToolBar* bar = env->addToolBar();

	video::ITexture* image = driver->getTexture("open.png");
	bar->addButton(GUI_ID_BUTTON_OPEN_MODEL, 0, L"Open a model", image, 0, false, true);

	image = driver->getTexture("tools.png");
	bar->addButton(GUI_ID_BUTTON_SHOW_TOOLBOX, 0, L"Open Toolset", image, 0, false, true);

	image = driver->getTexture("zip.png");
	bar->addButton(GUI_ID_BUTTON_SELECT_ARCHIVE, 0, L"Set Model Archive", image, 0, false, true);

	image = driver->getTexture("help.png");
	bar->addButton(GUI_ID_BUTTON_SHOW_ABOUT, 0, L"Open Help", image, 0, false, true);

	// create a combobox for texture filters

	gui::IGUIComboBox* box = env->addComboBox(core::rect<s32>(250, 4, 350, 23), bar, GUI_ID_TEXTUREFILTER);
	box->addItem(L"No filtering");
	box->addItem(L"Bilinear");
	box->addItem(L"Trilinear");
	box->addItem(L"Anisotropic");
	box->addItem(L"Isotropic");

	/*
	To make the editor look a little bit better, we disable transparent gui
	elements, and add an Irrlicht Engine logo. In addition, a text showing
	the current frames per second value is created and the window caption is
	changed.
	*/

	// disable alpha

	for (s32 i = 0; i < gui::EGDC_COUNT; ++i)
	{
		video::SColor col = env->getSkin()->getColor((gui::EGUI_DEFAULT_COLOR)i);
		col.setAlpha(255);
		env->getSkin()->setColor((gui::EGUI_DEFAULT_COLOR)i, col);
	}

	// add a tabcontrol

	createToolBox();

	// create fps text

	fpstext = env->addStaticText(L"",
		core::rect<s32>(400, 4, 570, 23), true, false, bar);

	postext = env->addStaticText(L"",
		core::rect<s32>(10, 50, 470, 80), false, false, 0, GUI_ID_POSITION_TEXT);
	postext->setVisible(false);

	// set window caption

	Caption += " - [";
	Caption += driver->getName();
	Caption += "]";
	Device->setWindowCaption(Caption.c_str());

	/*
	That's nearly the whole application. We simply show the about message
	box at start up, and load the first model. To make everything look
	better, a skybox is created and a user controlled camera, to make the
	application a little bit more interactive. Finally, everything is drawn
	in a standard drawing loop.
	*/

	// show about message box and load default model
	//if (argc == 1)
	//	showAboutText();
	loadModel(StartUpModelFile.c_str());

	// add skybox

	SkyBox = smgr->addSkyBoxSceneNode(
		driver->getTexture("irrlicht2_up.jpg"),
		driver->getTexture("irrlicht2_dn.jpg"),
		driver->getTexture("irrlicht2_lf.jpg"),
		driver->getTexture("irrlicht2_rt.jpg"),
		driver->getTexture("irrlicht2_ft.jpg"),
		driver->getTexture("irrlicht2_bk.jpg"));

	// add a camera scene node
	Camera[0] = smgr->addCameraSceneNodeMaya();
	Camera[0]->setFarValue(20000.f);
	// Maya cameras reposition themselves relative to their target, so target the location
	// where the mesh scene node is placed.
	Camera[0]->setTarget(core::vector3df(0, 30, 0));

	Camera[1] = smgr->addCameraSceneNodeFPS();
	Camera[1]->setFarValue(20000.f);
	Camera[1]->setPosition(core::vector3df(0, 0, -70));
	Camera[1]->setTarget(core::vector3df(0, 30, 0));

	setActiveCamera(Camera[0]);

	// load the irrlicht engine logo
	IGUIImage *img =
		env->addImage(driver->getTexture("irrlichtlogo2.png"),
			core::position2d<s32>(10, driver->getScreenSize().Height - 128));

	// lock the logo's edges to the bottom left corner of the screen
	img->setAlignment(EGUIA_UPPERLEFT, EGUIA_UPPERLEFT,
		EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);

	// remember state so we notice when the window does lose the focus
	//bool hasFocus = Device->isWindowFocused();

	// draw everything
	//while (Device->run() && driver)
	//{
		// Catch focus changes (workaround until Irrlicht has events for this)

		//bool focused = Device->isWindowFocused();
		//if (hasFocus && !focused)
		//	onKillFocus();
		//hasFocus = focused;

		//if (Device->isWindowActive())
		//{
			driver->beginScene(true, true, video::SColor(150, 50, 50, 50));

			smgr->drawAll();
			env->drawAll();

			driver->endScene();

			// update information about current frame-rate
			core::stringw str(L"FPS: ");
			str.append(core::stringw(driver->getFPS()));
			str += L" Tris: ";
			str.append(core::stringw(driver->getPrimitiveCountDrawn()));
			fpstext->setText(str.c_str());

			// update information about the active camera
			scene::ICameraSceneNode* cam = Device->getSceneManager()->getActiveCamera();
			str = L"Pos: ";
			str.append(core::stringw(cam->getPosition().X));
			str += L" ";
			str.append(core::stringw(cam->getPosition().Y));
			str += L" ";
			str.append(core::stringw(cam->getPosition().Z));
			str += L" Tgt: ";
			str.append(core::stringw(cam->getTarget().X));
			str += L" ";
			str.append(core::stringw(cam->getTarget().Y));
			str += L" ";
			str.append(core::stringw(cam->getTarget().Z));
			postext->setText(str.c_str());

			// update the tool dialog
			updateToolBox();
		//}
		//else
		//	Device->yield();
	//}


	//Device->drop();
	//return 0;
	return;

}

VisView::~VisView()
{
	mutex.lock();
	abort = true;
	condition.wakeOne();
	mutex.unlock();
}


void VisView::run() {

	forever
	{

		if (!Device->run())
			break;

		if (!driver)
			break;

		// Catch focus changes (workaround until Irrlicht has events for this)

		//bool focused = Device->isWindowFocused();
		//if (hasFocus && !focused)
		//	onKillFocus();
		//hasFocus = focused;

		//if (Device->isWindowActive())
		//{
		driver->beginScene(true, true, video::SColor(150, 50, 50, 50));

		smgr->drawAll();
		env->drawAll();

		//glBegin();

		//glEnd();

		driver->endScene();

		// update information about current frame-rate
		core::stringw str(L"FPS: ");
		str.append(core::stringw(driver->getFPS()));
		str += L" Tris: ";
		str.append(core::stringw(driver->getPrimitiveCountDrawn()));
		fpstext->setText(str.c_str());

		// update information about the active camera
		scene::ICameraSceneNode* cam = Device->getSceneManager()->getActiveCamera();
		str = L"Pos: ";
		str.append(core::stringw(cam->getPosition().X));
		str += L" ";
		str.append(core::stringw(cam->getPosition().Y));
		str += L" ";
		str.append(core::stringw(cam->getPosition().Z));
		str += L" Tgt: ";
		str.append(core::stringw(cam->getTarget().X));
		str += L" ";
		str.append(core::stringw(cam->getTarget().Y));
		str += L" ";
		str.append(core::stringw(cam->getTarget().Z));
		postext->setText(str.c_str());

		// update the tool dialog
		updateToolBox();
		//}
		//else
		//	Device->yield();
	}


	Device->drop();

	//return 0;
	return;

}

void VisView::stop() {


}

void VisView::setQTDialog(QTIrrlichtOrbifordVis * qt)
{
	m_qt.reset(qt);
}

void VisView::startVisView()
{
	//if (!isRunning()) {
	//	start(LowPriority);
	//}

	run();
}



/*
Toggle between various cameras
*/
void VisView::setActiveCamera(scene::ICameraSceneNode* newActive)
{
	if (0 == Device)
		return;

	scene::ICameraSceneNode * active = Device->getSceneManager()->getActiveCamera();
	active->setInputReceiverEnabled(false);

	newActive->setInputReceiverEnabled(true);
	Device->getSceneManager()->setActiveCamera(newActive);
}

/*
Set the skin transparency by changing the alpha values of all skin-colors
*/
void VisView::setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin)
{
	for (s32 i = 0; i<irr::gui::EGDC_COUNT; ++i)
	{
		video::SColor col = skin->getColor((EGUI_DEFAULT_COLOR)i);
		col.setAlpha(alpha);
		skin->setColor((EGUI_DEFAULT_COLOR)i, col);
	}
}

/*
Update the display of the model scaling
*/
void VisView::updateScaleInfo(scene::ISceneNode* model)
{
	IGUIElement* toolboxWnd = Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUI_ID_DIALOG_ROOT_WINDOW, true);
	if (!toolboxWnd)
		return;
	if (!model)
	{
		toolboxWnd->getElementFromId(GUI_ID_X_SCALE, true)->setText(L"-");
		toolboxWnd->getElementFromId(GUI_ID_Y_SCALE, true)->setText(L"-");
		toolboxWnd->getElementFromId(GUI_ID_Z_SCALE, true)->setText(L"-");
	}
	else
	{
		core::vector3df scale = model->getScale();
		toolboxWnd->getElementFromId(GUI_ID_X_SCALE, true)->setText(core::stringw(scale.X).c_str());
		toolboxWnd->getElementFromId(GUI_ID_Y_SCALE, true)->setText(core::stringw(scale.Y).c_str());
		toolboxWnd->getElementFromId(GUI_ID_Z_SCALE, true)->setText(core::stringw(scale.Z).c_str());
	}
}

/*
Function showAboutText() displays a messagebox with a caption and
a message text. The texts will be stored in the MessageText and Caption
variables at startup.
*/
void VisView::showAboutText()
{
	// create modal message box with the text
	// loaded from the xml file.
	Device->getGUIEnvironment()->addMessageBox(
		Caption.c_str(), MessageText.c_str());
}


/*
Function loadModel() loads a model and displays it using an
addAnimatedMeshSceneNode and the scene manager. Nothing difficult. It also
displays a short message box, if the model could not be loaded.
*/
void VisView::loadModel(const c8* fn)
{
	// modify the name if it a .pk3 file

	io::path filename(fn);

	io::path extension;
	core::getFileNameExtension(extension, filename);
	extension.make_lower();

	// if a texture is loaded apply it to the current model..
	if (extension == ".jpg" || extension == ".pcx" ||
		extension == ".png" || extension == ".ppm" ||
		extension == ".pgm" || extension == ".pbm" ||
		extension == ".psd" || extension == ".tga" ||
		extension == ".bmp" || extension == ".wal" ||
		extension == ".rgb" || extension == ".rgba")
	{
		video::ITexture * texture =
			Device->getVideoDriver()->getTexture(filename);
		if (texture && Model)
		{
			// always reload texture
			Device->getVideoDriver()->removeTexture(texture);
			texture = Device->getVideoDriver()->getTexture(filename);

			Model->setMaterialTexture(0, texture);
		}
		return;
	}
	// if a archive is loaded add it to the FileArchive..
	else if (extension == ".pk3" || extension == ".zip" || extension == ".pak" || extension == ".npk")
	{
		Device->getFileSystem()->addFileArchive(filename.c_str());
		return;
	}

	// load a model into the engine

	if (Model)
		Model->remove();

	Model = 0;

	if (extension == ".irr")
	{
		core::array<scene::ISceneNode*> outNodes;
		Device->getSceneManager()->loadScene(filename);
		Device->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANIMATED_MESH, outNodes);
		if (outNodes.size())
			Model = outNodes[0];
		return;
	}


	scene::IAnimatedMesh* m[1000];
	for (int i = 0; i< 1000; i++)
		m[i] = Device->getSceneManager()->getMesh(filename.c_str());

	if (!m)
	{
		// model could not be loaded

		if (StartUpModelFile != filename)
			Device->getGUIEnvironment()->addMessageBox(
				Caption.c_str(), L"The model could not be loaded. " \
				L"Maybe it is not a supported file format.");
		return;
	}

	// set default material properties

	scene::IAnimatedMeshSceneNode* animModel[1000];
	scene::ISceneNode* myModel[1000];

	for (int i = 0; i < 1000; i++)
	{
		if (Octree)
		{
			myModel[i] = Device->getSceneManager()->addOctreeSceneNode(m[i]->getMesh(0));
		}
		else
		{
			animModel[i] = Device->getSceneManager()->addAnimatedMeshSceneNode(m[i]);
			animModel[i]->setAnimationSpeed(30);
			myModel[i] = animModel[i];
			scene::ISceneNodeAnimator* anim = smgr->createRotationAnimator(core::vector3df(0, 1, 0));
			myModel[i]->addAnimator(anim);
			animModel[i]->setFrameLoop(0, 13);
			animModel[i]->setAnimationSpeed(15);

			anim->drop();
		}

		myModel[i]->setPosition(core::vector3df((i / 20) * 200, 0, (i % 20) * 200));
		myModel[i]->setMaterialFlag(video::EMF_LIGHTING, UseLight);
		myModel[i]->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, UseLight);
		//	Model->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		myModel[i]->setDebugDataVisible(scene::EDS_OFF);



	}
	// we need to uncheck the menu entries. would be cool to fake a menu event, but
	// that's not so simple. so we do it brute force
	gui::IGUIContextMenu* menu = (gui::IGUIContextMenu*)Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUI_ID_TOGGLE_DEBUG_INFO, true);
	if (menu)
		for (int item = 1; item < 6; ++item)
			menu->setItemChecked(item, false);

	for (int i = 0; i<1000; i++)
		updateScaleInfo(myModel[i]);

	Model = myModel[0];
}


/*
Function createToolBox() creates a toolbox window. In this simple mesh
viewer, this toolbox only contains a tab control with three edit boxes for
changing the scale of the displayed model.
*/
void VisView::createToolBox()
{
	// remove tool box if already there
	IGUIEnvironment* env = Device->getGUIEnvironment();
	IGUIElement* root = env->getRootGUIElement();
	IGUIElement* e = root->getElementFromId(GUI_ID_DIALOG_ROOT_WINDOW, true);
	if (e)
		e->remove();

	// create the toolbox window
	IGUIWindow* wnd = env->addWindow(core::rect<s32>(600, 45, 800, 480),
		false, L"Toolset", 0, GUI_ID_DIALOG_ROOT_WINDOW);

	// create tab control and tabs
	IGUITabControl* tab = env->addTabControl(
		core::rect<s32>(2, 20, 800 - 602, 480 - 7), wnd, true, true);

	IGUITab* t1 = tab->addTab(L"Config");

	// add some edit boxes and a button to tab one
	env->addStaticText(L"Scale:",
		core::rect<s32>(10, 20, 60, 45), false, false, t1);
	env->addStaticText(L"X:", core::rect<s32>(22, 48, 40, 66), false, false, t1);
	env->addEditBox(L"1.0", core::rect<s32>(40, 46, 130, 66), true, t1, GUI_ID_X_SCALE);
	env->addStaticText(L"Y:", core::rect<s32>(22, 82, 40, 96), false, false, t1);
	env->addEditBox(L"1.0", core::rect<s32>(40, 76, 130, 96), true, t1, GUI_ID_Y_SCALE);
	env->addStaticText(L"Z:", core::rect<s32>(22, 108, 40, 126), false, false, t1);
	env->addEditBox(L"1.0", core::rect<s32>(40, 106, 130, 126), true, t1, GUI_ID_Z_SCALE);

	env->addButton(core::rect<s32>(10, 134, 85, 165), t1, GUI_ID_BUTTON_SET_SCALE, L"Set");

	// quick scale buttons
	env->addButton(core::rect<s32>(65, 20, 95, 40), t1, GUI_ID_BUTTON_SCALE_MUL10, L"* 10");
	env->addButton(core::rect<s32>(100, 20, 130, 40), t1, GUI_ID_BUTTON_SCALE_DIV10, L"* 0.1");

	updateScaleInfo(Model);

	// add transparency control
	env->addStaticText(L"GUI Transparency Control:",
		core::rect<s32>(10, 200, 150, 225), true, false, t1);
	IGUIScrollBar* scrollbar = env->addScrollBar(true,
		core::rect<s32>(10, 225, 150, 240), t1, GUI_ID_SKIN_TRANSPARENCY);
	scrollbar->setMax(255);
	scrollbar->setPos(255);

	// add framerate control
	env->addStaticText(L":", core::rect<s32>(10, 240, 150, 265), true, false, t1);
	env->addStaticText(L"Framerate:",
		core::rect<s32>(12, 240, 75, 265), false, false, t1);
	// current frame info
	env->addStaticText(L"", core::rect<s32>(75, 240, 200, 265), false, false, t1,
		GUI_ID_ANIMATION_INFO);
	scrollbar = env->addScrollBar(true,
		core::rect<s32>(10, 265, 150, 280), t1, GUI_ID_SKIN_ANIMATION_FPS);
	scrollbar->setMax(MAX_FRAMERATE);
	scrollbar->setMin(-MAX_FRAMERATE);
	scrollbar->setPos(DEFAULT_FRAMERATE);
	scrollbar->setSmallStep(1);
}

/*
Function updateToolBox() is called each frame to update dynamic information in
the toolbox.
*/
void VisView::updateToolBox()
{
	IGUIEnvironment* env = Device->getGUIEnvironment();
	IGUIElement* root = env->getRootGUIElement();
	IGUIElement* dlg = root->getElementFromId(GUI_ID_DIALOG_ROOT_WINDOW, true);
	if (!dlg)
		return;

	// update the info we have about the animation of the model
	IGUIStaticText *  aniInfo = (IGUIStaticText *)(dlg->getElementFromId(GUI_ID_ANIMATION_INFO, true));
	if (aniInfo)
	{
		if (Model && scene::ESNT_ANIMATED_MESH == Model->getType())
		{
			scene::IAnimatedMeshSceneNode* animatedModel = (scene::IAnimatedMeshSceneNode*)Model;

			core::stringw str((s32)core::round_(animatedModel->getAnimationSpeed()));
			str += L" Frame: ";
			str += core::stringw((s32)animatedModel->getFrameNr());
			aniInfo->setText(str.c_str());
		}
		else
			aniInfo->setText(L"");
	}
}

void VisView::onKillFocus()
{
	// Avoid that the FPS-camera continues moving when the user presses alt-tab while 
	// moving the camera. 
	const core::list<scene::ISceneNodeAnimator*>& animators = Camera[1]->getAnimators();
	core::list<irr::scene::ISceneNodeAnimator*>::ConstIterator iter = animators.begin();
	while (iter != animators.end())
	{
		if ((*iter)->getType() == scene::ESNAT_CAMERA_FPS)
		{
			// we send a key-down event for all keys used by this animator
			scene::ISceneNodeAnimatorCameraFPS * fpsAnimator = static_cast<scene::ISceneNodeAnimatorCameraFPS*>(*iter);
			const core::array<SKeyMap>& keyMap = fpsAnimator->getKeyMap();
			for (irr::u32 i = 0; i< keyMap.size(); ++i)
			{
				irr::SEvent event;
				event.EventType = EET_KEY_INPUT_EVENT;
				event.KeyInput.Key = keyMap[i].KeyCode;
				event.KeyInput.PressedDown = false;
				fpsAnimator->OnEvent(event);
			}
		}
		++iter;
	}
}

/*
Function hasModalDialog() checks if we currently have a modal dialog open.
*/
bool VisView::hasModalDialog()
{
	if (!Device)
		return false;
	IGUIEnvironment* env = Device->getGUIEnvironment();
	IGUIElement * focused = env->getFocus();
	while (focused)
	{
		if (focused->isVisible() && focused->hasType(EGUIET_MODAL_SCREEN))
			return true;
		focused = focused->getParent();
	}
	return false;
}