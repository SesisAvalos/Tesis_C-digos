#ifndef MYLABEL_H
#define MYLABEL_H

#include <QtCore>
#include <QWidget>
#include <QLabel>

//Este codigo cambia la imagen en un QLabel a traves de un QImage en un entorno multi-hilo
class MyLabel : public QLabel {
  Q_OBJECT;
  public: 
    MyLabel(QWidget *parent = 0);
    ~MyLabel();

  public slots:
    void setImage(QImage);
    };
#endif

