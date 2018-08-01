#include "QTIrrlichtOrbifordVis.h"
#include "VisView.h"


QTIrrlichtOrbifordVis::QTIrrlichtOrbifordVis(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	m_visView.release();
}

void QTIrrlichtOrbifordVis::setVisView(VisView * vis)
{
	m_visView.reset(vis);
}

void QTIrrlichtOrbifordVis::on_pushButton_stopAnimations_clicked()
{
	core::vector3df scale(20.0, 20.0, 20.0);

	if (m_visView->Model)
		m_visView->Model->setScale(scale);
	m_visView->updateScaleInfo(m_visView->Model);

}
