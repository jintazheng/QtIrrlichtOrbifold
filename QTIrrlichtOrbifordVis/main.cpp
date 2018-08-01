#include "QTIrrlichtOrbifordVis.h"
#include <QtWidgets/QApplication>
#include "VisView.h"


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);


	QTIrrlichtOrbifordVis *w = new QTIrrlichtOrbifordVis();
	w->show();
	
	VisView *m_visView = new VisView(w);

	m_visView->startVisView();

	//m_visView->run();
	
	//m_visView->start();
	return a.exec();

}
