#include "QTIrrlichtOrbifordVis.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);


	QTIrrlichtOrbifordVis *mainWindow = new QTIrrlichtOrbifordVis();
	mainWindow->show();

	mainWindow->getIrrlichtWidget()->init();

	return a.exec();

}
