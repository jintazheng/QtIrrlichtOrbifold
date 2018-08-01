#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QTIrrlichtOrbifordVis.h"
#include "irrlichtWidget.h"
#include <QMessageBox>

#include <iostream>
#include <memory>

class VisView;

class QTIrrlichtOrbifordVis : public QMainWindow
{
	Q_OBJECT

public:
	QTIrrlichtOrbifordVis(QWidget *parent = Q_NULLPTR);

	void setVisView(VisView *vis);

	irrlichtWidget* getIrrlichtWidget() { return irrWidget; }

	bool eventFilter(QObject *target, QEvent *event);

private slots:
	
	void irrWidgetResize(QSize);

	void on_pushButton_stopAnimations_clicked();

    void on_pushButton_AddObj_clicked();

private:
	Ui::QTIrrlichtOrbifordVisClass ui;
	irrlichtWidget *irrWidget;
	//std::unique_ptr<VisView> m_visView;
};
