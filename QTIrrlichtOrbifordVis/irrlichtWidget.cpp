#include "irrlichtWidget.h"
#include <QtCore/QDebug>
#include "VisEventListener.h"

irrlichtWidget::irrlichtWidget(QWidget *parent)
{
	// Indicates that the widget wants to draw directly onto the screen. (From documentation : http://doc.qt.nokia.com/latest/qt.html)
	// Essential to have this or there will be nothing displayed
	setAttribute(Qt::WA_PaintOnScreen);
	// Indicates that the widget paints all its pixels when it receives a paint event.
	// Thus, it is not required for operations like updating, resizing, scrolling and focus changes to erase the widget before generating paint events.
	// Not sure this is required for the program to run properly, but it is here just incase.
	setAttribute(Qt::WA_OpaquePaintEvent);
	// Widget accepts focus by both tabbing and clicking
	setFocusPolicy(Qt::StrongFocus);
	// Not sure if this is necessary, but it was in the code I am basing this solution off of
	setAutoFillBackground(false);

	device = 0;
}

irrlichtWidget::~irrlichtWidget()
{
	if (device != 0)
	{
		device->closeDevice();
		device->drop();
	}
}

// Create the Irrlicht device and connect the signals and slots
void irrlichtWidget::init()
{
	// Make sure we can't create the device twice
	if (device != 0)
		return;

	//receiver = new MyEventReceiver(this);

	// Set all the device creation parameters
	SIrrlichtCreationParameters params;
	//params.AntiAlias = 0;
	params.Bits = 16;
	//params.DeviceType = EIDT_X11;
	//params.Doublebuffer = true;
	params.DriverType = EDT_OPENGL;
	params.EventReceiver = 0;
	params.Fullscreen = false;
	//params.HighPrecisionFPU = false;
	params.IgnoreInput = false;
	//params.LoggingLevel = ELL_INFORMATION;
	params.Stencilbuffer = false;
	//params.Stereobuffer = false;
	params.Vsync = false;
	// Specify which window/widget to render to
	params.WindowId = reinterpret_cast<void*>(winId());
	params.WindowSize.Width = 800;
	params.WindowSize.Height = 600;
	//params.WithAlphaChannel = false;
	//params.ZBufferBits = 16;

	 //Create the Irrlicht Device with the previously specified parameters
	device = createDeviceEx(params);

	//video::E_DRIVER_TYPE driverType = video::E_DRIVER_TYPE::EDT_OPENGL;
	//device = createDevice(driverType, core::dimension2d<u32>(width(), height()),
	//	16, false, false, false, receiver);

	device->setResizable(true);

	//if (device)
	//{
	//	// Create a camera so we can view the scene
	//	camera = device->getSceneManager()->addCameraSceneNode(0, vector3df(0, 30, -40), vector3df(0, 5, 0));
	//}

	// Connect the update signal (updateIrrlichtQuery) to the update slot (updateIrrlicht)
	connect(this, SIGNAL(updateIrrlichtQuery(IrrlichtDevice*)), this, SLOT(updateIrrlicht(IrrlichtDevice*)));

	// Start a timer. A timer with setting 0 will update as often as possible.
	startTimer(0);

	//grabKeyboard();
	//setMouseTracking(true);

	//IrrDisplay->installEventFilter(this);//used for mouse leave and enter events (can be used for any widget)

	initalizeScene();
}

void irrlichtWidget::initalizeScene()
{
	// Make sure the Irrlicht Device exists before trying to use it
	if (getIrrlichtDevice())
	{
		smgr = getIrrlichtDevice()->getSceneManager();
		driver = getIrrlichtDevice()->getVideoDriver();


		env = getIrrlichtDevice()->getGUIEnvironment();
		smgr = getIrrlichtDevice()->getSceneManager();
		smgr->getParameters()->setAttribute(scene::COLLADA_CREATE_SCENE_INSTANCES, true);

		driver->setTextureCreationFlag(video::ETCF_ALWAYS_32_BIT, true);

		smgr->addLightSceneNode(0, core::vector3df(200, 200, 200),
			video::SColorf(1.0f, 1.0f, 1.0f), 2000);
		smgr->setAmbientLight(video::SColorf(0.3f, 0.3f, 0.3f));
		// add our media directory as "search path"
		getIrrlichtDevice()->getFileSystem()->addFileArchive("./../media/");

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

		io::IXMLReader* xml = getIrrlichtDevice()->getFileSystem()->createXMLReader(L"config.xml");

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
		postext->setVisible(true);

		// set window caption

		Caption += " - [";
		Caption += driver->getName();
		Caption += "]";
		getIrrlichtDevice()->setWindowCaption(Caption.c_str());

		// show about message box and load default model
		//if (argc == 1)
		//	showAboutText();
		loadModel(StartUpModelFile.c_str());

		// add a camera scene node
		Camera[0] = smgr->addCameraSceneNode();
		Camera[0]->setFarValue(20000.f);
		// Maya cameras reposition themselves relative to their target, so target the location
		// where the mesh scene node is placed.
		Camera[0]->setPosition(core::vector3df(100, 1000, 1000));
		Camera[0]->setTarget(core::vector3df(0, 100, 0));

		Camera[1] = smgr->addCameraSceneNodeFPS();
		Camera[1]->setFarValue(20000.f);
		Camera[1]->setPosition(core::vector3df(100, 1000, 1100));
		Camera[1]->setTarget(core::vector3df(0, 100, 0));

		setActiveCamera(Camera[0]);

		// load the irrlicht engine logo
		IGUIImage *img =
			env->addImage(driver->getTexture("irrlichtlogo2.png"),
				core::position2d<s32>(10, driver->getScreenSize().Height - 128));

		// lock the logo's edges to the bottom left corner of the screen
		img->setAlignment(EGUIA_UPPERLEFT, EGUIA_UPPERLEFT,
			EGUIA_LOWERRIGHT, EGUIA_LOWERRIGHT);

	}
}

IrrlichtDevice* irrlichtWidget::getIrrlichtDevice()
{
	return device;
}

void irrlichtWidget::paintEvent(QPaintEvent* event)
{
	if (device != 0)
	{
		emit updateIrrlichtQuery(device);
	}
}

void irrlichtWidget::timerEvent(QTimerEvent* event)
{
	// Emit the render signal each time the timer goes off
	if (device != 0)
	{
		emit updateIrrlichtQuery(device);
	}

	event->accept();
}

void irrlichtWidget::resizeEvent(QResizeEvent* event)
{
	if (device != 0)
	{
		dimension2d<u32> widgetSize;
		widgetSize.Width = event->size().width();
		widgetSize.Height = event->size().height();
		device->getVideoDriver()->OnResize(widgetSize);

		ICameraSceneNode *cam = device->getSceneManager()->getActiveCamera();
		if (cam != 0)
		{
			cam->setAspectRatio((f32)widgetSize.Height / (f32)widgetSize.Width);
		}
	}

	QWidget::resizeEvent(event);
}

struct SIrrlichtKey
{
	irr::EKEY_CODE code;
	wchar_t ch;
};

SIrrlichtKey convertToIrrlichtKey(int key)
{
	SIrrlichtKey irrKey;
	irrKey.code = (irr::EKEY_CODE)(0);
	irrKey.ch = (wchar_t)(0);


	// Letters A..Z and numbers 0..9 are mapped directly
	if ((key >= Qt::Key_A && key <= Qt::Key_Z) || (key >= Qt::Key_0 && key <= Qt::Key_9))
	{
		irrKey.code = (irr::EKEY_CODE)(key);
		irrKey.ch = (wchar_t)(key);
	}
	else


		// Dang, map keys individually
#define MAP_QT_IRRLICHT_KEY(QT_KEY,IRRLICHT_KEY) \
                                                case QT_KEY: \
                                                    irrKey.code = IRRLICHT_KEY; \
                                                    break;


		switch (key)
		{
			MAP_QT_IRRLICHT_KEY(Qt::Key_Escape, irr::KEY_ESCAPE)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Tab, irr::KEY_TAB)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Back, irr::KEY_BACK)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Return, irr::KEY_RETURN)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Enter, irr::KEY_RETURN)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Insert, irr::KEY_INSERT)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Delete, irr::KEY_DELETE)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Pause, irr::KEY_PAUSE)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Print, irr::KEY_PRINT)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_SysReq, irr::KEY_S
				MAP_QT_IRRLICHT_KEY(Qt::Key_Clear, irr::KEY_CLEAR)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Home, irr::KEY_HOME)
				MAP_QT_IRRLICHT_KEY(Qt::Key_End, irr::KEY_END)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Left, irr::KEY_LEFT)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Up, irr::KEY_UP)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Right, irr::KEY_RIGHT)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Down, irr::KEY_DOWN)
				MAP_QT_IRRLICHT_KEY(Qt::Key_PageUp, irr::KEY_PRIOR)
				MAP_QT_IRRLICHT_KEY(Qt::Key_PageDown, irr::KEY_NEXT)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Shift, irr::KEY_SHIFT)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Control, irr::KEY_CONTROL)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Meta, irr::KEY_ME
				MAP_QT_IRRLICHT_KEY(Qt::Key_Alt, irr::KEY_MENU)
				MAP_QT_IRRLICHT_KEY(Qt::Key_CapsLock, irr::KEY_CAPITAL)
				MAP_QT_IRRLICHT_KEY(Qt::Key_NumLock, irr::KEY_NUMLOCK)
				MAP_QT_IRRLICHT_KEY(Qt::Key_ScrollLock, irr::KEY_SCROLL)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F1, irr::KEY_F1)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F2, irr::KEY_F2)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F3, irr::KEY_F3)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F4, irr::KEY_F4)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F5, irr::KEY_F5)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F6, irr::KEY_F6)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F7, irr::KEY_F7)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F8, irr::KEY_F8)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F9, irr::KEY_F9)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F10, irr::KEY_F10)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F11, irr::KEY_F11)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F12, irr::KEY_F12)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F13, irr::KEY_F13)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F14, irr::KEY_F14)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F15, irr::KEY_F15)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F16, irr::KEY_F16)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F17, irr::KEY_F17)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F18, irr::KEY_F18)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F19, irr::KEY_F19)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F20, irr::KEY_F20)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F21, irr::KEY_F21)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F22, irr::KEY_F22)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F23, irr::KEY_F23)
				MAP_QT_IRRLICHT_KEY(Qt::Key_F24, irr::KEY_F24)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F25, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F26, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F27, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F28, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F29, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F30, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F31, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F32, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F33, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F34, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_F35, irr::KEY_F
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Super_L, irr::KEY_SU
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Super_R
				MAP_QT_IRRLICHT_KEY(Qt::Key_Menu, irr::KEY_MENU)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Hyper_L
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Hyper_R
				MAP_QT_IRRLICHT_KEY(Qt::Key_Help, irr::KEY_HELP)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Direction_L
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Direction_R
				MAP_QT_IRRLICHT_KEY(Qt::Key_Space, irr::KEY_SPACE)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Any, irr::KEY_SPACE)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Exclam
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_QuoteDbl
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_NumberSign
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Dollar
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Percent
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Ampersand
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Apostrophe
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_ParenLeft
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_ParenRight
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Asterisk, irr::
				MAP_QT_IRRLICHT_KEY(Qt::Key_Plus, irr::KEY_PLUS)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Comma, irr::KEY_COMMA)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Minus, irr::KEY_MINUS)
				MAP_QT_IRRLICHT_KEY(Qt::Key_Period, irr::KEY_PERIOD)
				//            MAP_QT_IRRLICHT_KEY(Qt::Key_Slash,irr::KEY_SEPARATOR)




		}
#undef MAP_QT_IRRLICHT_KEY
	return irrKey;
}


void irrlichtWidget::sendKeyEventToIrrlicht(QKeyEvent* event, bool pressedDown)
{
	irr::SEvent irrEvent;


	irrEvent.EventType = irr::EET_KEY_INPUT_EVENT;


	SIrrlichtKey irrKey = convertToIrrlichtKey(event->key());


	if (irrKey.code == 0) return; // Could not find a match for this key


	irrEvent.KeyInput.Key = irrKey.code;
	irrEvent.KeyInput.Control = ((event->modifiers() & Qt::ControlModifier) != 0);
	irrEvent.KeyInput.Shift = ((event->modifiers() & Qt::ShiftModifier) != 0);
	irrEvent.KeyInput.Char = irrKey.ch;
	irrEvent.KeyInput.PressedDown = pressedDown;


	getIrrlichtDevice()->postEventFromUser(irrEvent);
}

void irrlichtWidget::sendMouseEventToIrrlicht(QMouseEvent* event, bool pressedDown)
{
	irr::SEvent irrEvent;


	irrEvent.EventType = irr::EET_MOUSE_INPUT_EVENT;


	switch (event->button())
	{
	case Qt::LeftButton:
		irrEvent.MouseInput.Event = pressedDown ? irr::EMIE_LMOUSE_PRESSED_DOWN : irr::EMIE_LMOUSE_LEFT_UP;
		m_mousebutton_state = pressedDown ? MOUSELEFT : NOMOUSEBUTTON;
		break;


	case Qt::MidButton:
		irrEvent.MouseInput.Event = pressedDown ? irr::EMIE_MMOUSE_PRESSED_DOWN : irr::EMIE_MMOUSE_LEFT_UP;
		m_mousebutton_state = pressedDown ? MOUSEMIDDLE : NOMOUSEBUTTON;
		break;


	case Qt::RightButton:
		irrEvent.MouseInput.Event = pressedDown ? irr::EMIE_RMOUSE_PRESSED_DOWN : irr::EMIE_RMOUSE_LEFT_UP;
		m_mousebutton_state = pressedDown ? MOUSERIGHT : NOMOUSEBUTTON;
		break;


	default:
		return; // Cannot handle this mouse event
	}

	//irrEvent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;

	irrEvent.MouseInput.X = event->x();
	irrEvent.MouseInput.Y = event->y();
	irrEvent.MouseInput.Wheel = 0.0f; // Zero is better than undefined
	irrEvent.MouseInput.Control = ((event->modifiers() & Qt::ControlModifier) != 0);
	irrEvent.MouseInput.Shift = ((event->modifiers() & Qt::ShiftModifier) != 0);

	getIrrlichtDevice()->postEventFromUser(irrEvent);
}

void irrlichtWidget::mouseDoubleClickEvent(QMouseEvent * event)
{
	if (getIrrlichtDevice() != 0)
	{
		sendMouseEventToIrrlicht(event, true);
	}
}

void irrlichtWidget::mouseMoveEvent(QMouseEvent * event)
{
	irr::SEvent irrEvent;

	QPoint currLoc = event->pos();
	QPoint dloc = currLoc - lastLoc;
	lastLoc = currLoc;

	irrEvent.EventType = irr::EET_MOUSE_INPUT_EVENT;

	if (device != 0)
	{
		irrEvent.MouseInput.Event = irr::EMIE_MOUSE_MOVED;

		irrEvent.MouseInput.X = event->x();
		irrEvent.MouseInput.Y = event->y();
		irrEvent.MouseInput.Wheel = 0.0f; // Zero is better than undefined

		if(m_mousebutton_state == MOUSEBUTTONSTATE::MOUSELEFT)
			irrEvent.MouseInput.ButtonStates = EMBSM_LEFT;
		if (m_mousebutton_state == MOUSEBUTTONSTATE::MOUSERIGHT)
			irrEvent.MouseInput.ButtonStates = EMBSM_RIGHT;
		if (m_mousebutton_state == MOUSEBUTTONSTATE::MOUSEMIDDLE)
			irrEvent.MouseInput.ButtonStates = EMBSM_MIDDLE;

		device->postEventFromUser(irrEvent);
	}


	event->ignore();
}

void irrlichtWidget::mousePressEvent(QMouseEvent * event)
{
	lastLoc = event->pos();

	if (device != 0)
	{
		sendMouseEventToIrrlicht(event, true);
	}
	event->ignore();
}

void irrlichtWidget::mouseReleaseEvent(QMouseEvent * event)
{
	if (getIrrlichtDevice() != 0)
	{
		sendMouseEventToIrrlicht(event, false);
	}
	event->ignore();
}

void irrlichtWidget::wheelEvent(QWheelEvent * event)
{
	if (getIrrlichtDevice() != 0 && event->orientation() == Qt::Vertical)
	{
		irr::SEvent irrEvent;


		irrEvent.EventType = irr::EET_MOUSE_INPUT_EVENT;


		irrEvent.MouseInput.Event = irr::EMIE_MOUSE_WHEEL;
		irrEvent.MouseInput.X = 0; // We don't know these,
		irrEvent.MouseInput.Y = 0; // but better zero them instead of letting them be undefined
		irrEvent.MouseInput.Wheel = event->delta() / 120.0f;


		getIrrlichtDevice()->postEventFromUser(irrEvent);
	}
}


void irrlichtWidget::keyPressEvent(QKeyEvent * event)
{
	if (getIrrlichtDevice() != 0)
	{
		sendKeyEventToIrrlicht(event, false);
	}
	//event->ignore();
}

void irrlichtWidget::keyReleaseEvent(QKeyEvent * event)
{
	// Don't handle keys if we have a modal dialog open as it would lead 
	// to unexpected application behaviour for the user.
	if (hasModalDialog())
		return;

	if (event->key() == Qt::Key_Escape)
	{
		if (getIrrlichtDevice())
		{
			scene::ICameraSceneNode * camera =
				getIrrlichtDevice()->getSceneManager()->getActiveCamera();
			if (camera)
			{
				camera->setInputReceiverEnabled(!camera->isInputReceiverEnabled());
			}
			return;
		}
	}
	else if (event->key() == Qt::Key_F1)
	{
		if (getIrrlichtDevice())
		{
			IGUIElement* elem =getIrrlichtDevice()->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUI_ID_POSITION_TEXT);
			if (elem)
				elem->setVisible(!elem->isVisible());
		}
	}
	//else if (event->key() == Qt::Key_M)
	//{
	//	if (getIrrlichtDevice())
	//		getIrrlichtDevice()->minimizeWindow();
	//}
	else if (event->key() == Qt::Key_L)
	{
		UseLight = !UseLight;
		if (Model)
		{
			Model->setMaterialFlag(video::EMF_LIGHTING, UseLight);
			Model->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, UseLight);
		}
	}
	return;

	if (getIrrlichtDevice() != 0)
	{
		sendKeyEventToIrrlicht(event, true);
	}
	//event->ignore();
}




void irrlichtWidget::updateIrrlicht(irr::IrrlichtDevice* device)
{
	if (device != 0)
	{
		device->getTimer()->tick();

		SColor color(255, 0, 0, 0);

		device->getVideoDriver()->beginScene(true, true, color);
		device->getSceneManager()->drawAll();
		//env->drawAll();

		device->getVideoDriver()->endScene();

		// update information about current frame-rate
		core::stringw str(L"FPS: ");
		str.append(core::stringw(device->getVideoDriver()->getFPS()));
		str += L" Tris: ";
		str.append(core::stringw(device->getVideoDriver()->getPrimitiveCountDrawn()));
		fpstext->setText(str.c_str());

		// update information about the active camera
		scene::ICameraSceneNode* cam = device->getSceneManager()->getActiveCamera();
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
	}

}


/*
Toggle between various cameras
*/
void irrlichtWidget::setActiveCamera(scene::ICameraSceneNode* newActive)
{
	if (0 == getIrrlichtDevice())
		return;

	scene::ICameraSceneNode * active = getIrrlichtDevice()->getSceneManager()->getActiveCamera();
	active->setInputReceiverEnabled(false);

	newActive->setInputReceiverEnabled(true);
	getIrrlichtDevice()->getSceneManager()->setActiveCamera(newActive);
}

/*
Set the skin transparency by changing the alpha values of all skin-colors
*/
void irrlichtWidget::setSkinTransparency(s32 alpha, irr::gui::IGUISkin * skin)
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
void irrlichtWidget::updateScaleInfo(scene::ISceneNode* model)
{
	IGUIElement* toolboxWnd = getIrrlichtDevice()->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUI_ID_DIALOG_ROOT_WINDOW, true);
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
void irrlichtWidget::showAboutText()
{
	// create modal message box with the text
	// loaded from the xml file.
	getIrrlichtDevice()->getGUIEnvironment()->addMessageBox(
		Caption.c_str(), MessageText.c_str());
}


/*
Function loadModel() loads a model and displays it using an
addAnimatedMeshSceneNode and the scene manager. Nothing difficult. It also
displays a short message box, if the model could not be loaded.
*/
void irrlichtWidget::loadModel(const c8* fn)
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
			getIrrlichtDevice()->getVideoDriver()->getTexture(filename);
		if (texture && Model)
		{
			// always reload texture
			getIrrlichtDevice()->getVideoDriver()->removeTexture(texture);
			texture = getIrrlichtDevice()->getVideoDriver()->getTexture(filename);

			Model->setMaterialTexture(0, texture);
		}
		return;
	}
	// if a archive is loaded add it to the FileArchive..
	else if (extension == ".pk3" || extension == ".zip" || extension == ".pak" || extension == ".npk")
	{
		getIrrlichtDevice()->getFileSystem()->addFileArchive(filename.c_str());
		return;
	}

	// load a model into the engine

	if (Model)
		Model->remove();

	Model = 0;

	if (extension == ".irr")
	{
		core::array<scene::ISceneNode*> outNodes;
		getIrrlichtDevice()->getSceneManager()->loadScene(filename);
		getIrrlichtDevice()->getSceneManager()->getSceneNodesFromType(scene::ESNT_ANIMATED_MESH, outNodes);
		if (outNodes.size())
			Model = outNodes[0];
		return;
	}


	scene::IAnimatedMesh* m[1000];
	for (int i = 0; i< 1000; i++)
		m[i] = getIrrlichtDevice()->getSceneManager()->getMesh(filename.c_str());

	if (!m)
	{
		// model could not be loaded

		if (StartUpModelFile != filename)
			getIrrlichtDevice()->getGUIEnvironment()->addMessageBox(
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
			myModel[i] = getIrrlichtDevice()->getSceneManager()->addOctreeSceneNode(m[i]->getMesh(0));
		}
		else
		{
			animModel[i] = getIrrlichtDevice()->getSceneManager()->addAnimatedMeshSceneNode(m[i]);
			myModel[i] = animModel[i];
			//scene::ISceneNodeAnimator* anim = getIrrlichtDevice()->getSceneManager()->createRotationAnimator(core::vector3df(0, 1, 0));
			//myModel[i]->addAnimator(anim);
			//animModel[i]->setFrameLoop(0, 13);
			//animModel[i]->setAnimationSpeed(15);

			//anim->drop();
		}

		myModel[i]->setPosition(core::vector3df((i / 20) * 200, 0, (i % 20) * 200));
		myModel[i]->setMaterialFlag(video::EMF_LIGHTING, UseLight);
		myModel[i]->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, UseLight);
		//	Model->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		myModel[i]->setDebugDataVisible(scene::EDS_OFF);



	}
	// we need to uncheck the menu entries. would be cool to fake a menu event, but
	// that's not so simple. so we do it brute force
	gui::IGUIContextMenu* menu = (gui::IGUIContextMenu*)getIrrlichtDevice()->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUI_ID_TOGGLE_DEBUG_INFO, true);
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
void irrlichtWidget::createToolBox()
{
	// remove tool box if already there
	IGUIEnvironment* env = getIrrlichtDevice()->getGUIEnvironment();
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
void irrlichtWidget::updateToolBox()
{
	IGUIEnvironment* env = getIrrlichtDevice()->getGUIEnvironment();
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

void irrlichtWidget::onKillFocus()
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
bool irrlichtWidget::hasModalDialog()
{
	if (!getIrrlichtDevice())
		return false;
	IGUIEnvironment* env = getIrrlichtDevice()->getGUIEnvironment();
	IGUIElement * focused = env->getFocus();
	while (focused)
	{
		if (focused->isVisible() && focused->hasType(EGUIET_MODAL_SCREEN))
			return true;
		focused = focused->getParent();
	}
	return false;
}