#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QTIrrlichtOrbifordVis.h"
#include <iostream>
#include <memory>

class VisView;

class QTIrrlichtOrbifordVis : public QMainWindow
{
	Q_OBJECT

public:
	QTIrrlichtOrbifordVis(QWidget *parent = Q_NULLPTR);

	void setVisView(VisView *vis);

private slots:
    void on_pushButton_stopAnimations_clicked();

private:
	Ui::QTIrrlichtOrbifordVisClass ui;
	
	std::unique_ptr<VisView> m_visView;
};
