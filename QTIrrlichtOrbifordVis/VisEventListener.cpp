#include "VisEventListener.h"
#include "VisView.h"


MyEventReceiver::MyEventReceiver(VisView *vis) {

	m_visView = vis;

}

bool MyEventReceiver::OnEvent(const SEvent& event)
{
	// Escape swaps Camera Input
	if (event.EventType == EET_KEY_INPUT_EVENT &&
		event.KeyInput.PressedDown == false)
	{
		if (OnKeyUp(event.KeyInput.Key))
			return true;
	}

	if (event.EventType == EET_GUI_EVENT)
	{
		s32 id = event.GUIEvent.Caller->getID();
		IGUIEnvironment* env = m_visView->Device->getGUIEnvironment();

		switch (event.GUIEvent.EventType)
		{
		case EGET_MENU_ITEM_SELECTED:
			// a menu item was clicked
			OnMenuItemSelected((IGUIContextMenu*)event.GUIEvent.Caller);
			break;

		case EGET_FILE_SELECTED:
		{
			// load the model file, selected in the file open dialog
			IGUIFileOpenDialog* dialog =
				(IGUIFileOpenDialog*)event.GUIEvent.Caller;
			m_visView->loadModel(core::stringc(dialog->getFileName()).c_str());
		}
		break;

		case EGET_SCROLL_BAR_CHANGED:

			// control skin transparency
			if (id == GUI_ID_SKIN_TRANSPARENCY)
			{
				const s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
				m_visView->setSkinTransparency(pos, env->getSkin());
			}
			// control animation speed
			else if (id == GUI_ID_SKIN_ANIMATION_FPS)
			{
				const s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
				if (scene::ESNT_ANIMATED_MESH == m_visView->Model->getType())
					((scene::IAnimatedMeshSceneNode*)m_visView->Model)->setAnimationSpeed((f32)pos);
			}
			break;

		case EGET_COMBO_BOX_CHANGED:

			// control anti-aliasing/filtering
			if (id == GUI_ID_TEXTUREFILTER)
			{
				OnTextureFilterSelected((IGUIComboBox*)event.GUIEvent.Caller);
			}
			break;

		case EGET_BUTTON_CLICKED:

			switch (id)
			{
			case GUI_ID_BUTTON_SET_SCALE:
			{
				// set scale
				gui::IGUIElement* root = env->getRootGUIElement();
				core::vector3df scale;
				core::stringc s;

				s = root->getElementFromId(GUI_ID_X_SCALE, true)->getText();
				scale.X = (f32)atof(s.c_str());
				s = root->getElementFromId(GUI_ID_Y_SCALE, true)->getText();
				scale.Y = (f32)atof(s.c_str());
				s = root->getElementFromId(GUI_ID_Z_SCALE, true)->getText();
				scale.Z = (f32)atof(s.c_str());

				if (m_visView->Model)
					m_visView->Model->setScale(scale);
				m_visView->updateScaleInfo(m_visView->Model);
			}
			break;
			case GUI_ID_BUTTON_SCALE_MUL10:
				if (m_visView->Model)
					m_visView->Model->setScale(m_visView->Model->getScale()*10.f);
				m_visView->updateScaleInfo(m_visView->Model);
				break;
			case GUI_ID_BUTTON_SCALE_DIV10:
				if (m_visView->Model)
					m_visView->Model->setScale(m_visView->Model->getScale()*0.1f);
				m_visView->updateScaleInfo(m_visView->Model);
				break;
			case GUI_ID_BUTTON_OPEN_MODEL:
				env->addFileOpenDialog(L"Please select a model file to open");
				break;
			case GUI_ID_BUTTON_SHOW_ABOUT:
				m_visView->showAboutText();
				break;
			case GUI_ID_BUTTON_SHOW_TOOLBOX:
				m_visView->createToolBox();
				break;
			case GUI_ID_BUTTON_SELECT_ARCHIVE:
				env->addFileOpenDialog(L"Please select your game archive/directory");
				break;
			}

			break;
		default:
			break;
		}
	}

	return false;
}


/*
Handle key-up events
*/
bool MyEventReceiver::OnKeyUp(irr::EKEY_CODE keyCode)
{
	// Don't handle keys if we have a modal dialog open as it would lead 
	// to unexpected application behaviour for the user.
	if (m_visView->hasModalDialog())
		return false;

	if (keyCode == irr::KEY_ESCAPE)
	{
		if (m_visView->Device)
		{
			scene::ICameraSceneNode * camera =
				m_visView->Device->getSceneManager()->getActiveCamera();
			if (camera)
			{
				camera->setInputReceiverEnabled(!camera->isInputReceiverEnabled());
			}
			return true;
		}
	}
	else if (keyCode == irr::KEY_F1)
	{
		if (m_visView->Device)
		{
			IGUIElement* elem = m_visView->Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId(GUI_ID_POSITION_TEXT);
			if (elem)
				elem->setVisible(!elem->isVisible());
		}
	}
	else if (keyCode == irr::KEY_KEY_M)
	{
		if (m_visView->Device)
			m_visView->Device->minimizeWindow();
	}
	else if (keyCode == irr::KEY_KEY_L)
	{
		m_visView->UseLight = !m_visView->UseLight;
		if (m_visView->Model)
		{
			m_visView->Model->setMaterialFlag(video::EMF_LIGHTING, m_visView->UseLight);
			m_visView->Model->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, m_visView->UseLight);
		}
	}
	return false;
}


/*
Handle "menu item clicked" events.
*/
void MyEventReceiver::OnMenuItemSelected(IGUIContextMenu* menu)
{
	s32 id = menu->getItemCommandId(menu->getSelectedItem());
	IGUIEnvironment* env = m_visView->Device->getGUIEnvironment();

	switch (id)
	{
	case GUI_ID_OPEN_MODEL: // FilOnButtonSetScalinge -> Open Model
		env->addFileOpenDialog(L"Please select a model file to open");
		break;
	case GUI_ID_SET_MODEL_ARCHIVE: // File -> Set Model Archive
		env->addFileOpenDialog(L"Please select your game archive/directory");
		break;
	case GUI_ID_LOAD_AS_OCTREE: // File -> LoadAsOctree
		m_visView->Octree = !m_visView->Octree;
		menu->setItemChecked(menu->getSelectedItem(), m_visView->Octree);
		break;
	case GUI_ID_QUIT: // File -> Quit
		m_visView->Device->closeDevice();
		break;
	case GUI_ID_SKY_BOX_VISIBLE: // View -> Skybox
		menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
		m_visView->SkyBox->setVisible(!m_visView->SkyBox->isVisible());
		break;
	case GUI_ID_DEBUG_OFF: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem() + 1, false);
		menu->setItemChecked(menu->getSelectedItem() + 2, false);
		menu->setItemChecked(menu->getSelectedItem() + 3, false);
		menu->setItemChecked(menu->getSelectedItem() + 4, false);
		menu->setItemChecked(menu->getSelectedItem() + 5, false);
		menu->setItemChecked(menu->getSelectedItem() + 6, false);
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible(scene::EDS_OFF);
		break;
	case GUI_ID_DEBUG_BOUNDING_BOX: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(m_visView->Model->isDebugDataVisible() ^ scene::EDS_BBOX));
		break;
	case GUI_ID_DEBUG_NORMALS: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(m_visView->Model->isDebugDataVisible() ^ scene::EDS_NORMALS));
		break;
	case GUI_ID_DEBUG_SKELETON: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(m_visView->Model->isDebugDataVisible() ^ scene::EDS_SKELETON));
		break;
	case GUI_ID_DEBUG_WIRE_OVERLAY: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(m_visView->Model->isDebugDataVisible() ^ scene::EDS_MESH_WIRE_OVERLAY));
		break;
	case GUI_ID_DEBUG_HALF_TRANSPARENT: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(m_visView->Model->isDebugDataVisible() ^ scene::EDS_HALF_TRANSPARENCY));
		break;
	case GUI_ID_DEBUG_BUFFERS_BOUNDING_BOXES: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem(), !menu->isItemChecked(menu->getSelectedItem()));
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(m_visView->Model->isDebugDataVisible() ^ scene::EDS_BBOX_BUFFERS));
		break;
	case GUI_ID_DEBUG_ALL: // View -> Debug Information
		menu->setItemChecked(menu->getSelectedItem() - 1, true);
		menu->setItemChecked(menu->getSelectedItem() - 2, true);
		menu->setItemChecked(menu->getSelectedItem() - 3, true);
		menu->setItemChecked(menu->getSelectedItem() - 4, true);
		menu->setItemChecked(menu->getSelectedItem() - 5, true);
		menu->setItemChecked(menu->getSelectedItem() - 6, true);
		if (m_visView->Model)
			m_visView->Model->setDebugDataVisible(scene::EDS_FULL);
		break;
	case GUI_ID_ABOUT: // Help->About
		m_visView->showAboutText();
		break;
	case GUI_ID_MODEL_MATERIAL_SOLID: // View -> Material -> Solid
		if (m_visView->Model)
			m_visView->Model->setMaterialType(video::EMT_SOLID);
		break;
	case GUI_ID_MODEL_MATERIAL_TRANSPARENT: // View -> Material -> Transparent
		if (m_visView->Model)
			m_visView->Model->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
		break;
	case GUI_ID_MODEL_MATERIAL_REFLECTION: // View -> Material -> Reflection
		if (m_visView->Model)
			m_visView->Model->setMaterialType(video::EMT_SPHERE_MAP);
		break;

	case GUI_ID_CAMERA_MAYA:
		m_visView->setActiveCamera(m_visView->Camera[0]);
		break;
	case GUI_ID_CAMERA_FIRST_PERSON:
		m_visView->setActiveCamera(m_visView->Camera[1]);
		break;
	}
}

/*
Handle the event that one of the texture-filters was selected in the corresponding combobox.
*/
void MyEventReceiver::OnTextureFilterSelected(IGUIComboBox* combo)
{
	s32 pos = combo->getSelected();
	switch (pos)
	{
	case 0:
		if (m_visView->Model)
		{
			m_visView->Model->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
			m_visView->Model->setMaterialFlag(video::EMF_TRILINEAR_FILTER, false);
			m_visView->Model->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, false);
		}
		break;
	case 1:
		if (m_visView->Model)
		{
			m_visView->Model->setMaterialFlag(video::EMF_BILINEAR_FILTER, true);
			m_visView->Model->setMaterialFlag(video::EMF_TRILINEAR_FILTER, false);
		}
		break;
	case 2:
		if (m_visView->Model)
		{
			m_visView->Model->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
			m_visView->Model->setMaterialFlag(video::EMF_TRILINEAR_FILTER, true);
		}
		break;
	case 3:
		if (m_visView->Model)
		{
			m_visView->Model->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, true);
		}
		break;
	case 4:
		if (m_visView->Model)
		{
			m_visView->Model->setMaterialFlag(video::EMF_ANISOTROPIC_FILTER, false);
		}
		break;
	}
}