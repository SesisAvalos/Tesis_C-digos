//Se manda a llamar a MyLabel.h
#include "MyLabel.h"
//Se crea el constructor de la clase MyLabel
MyLabel::MyLabel(QWidget *parent) : QLabel(parent)
{
}.
//Destructor de la clase MyLabel
MyLabel::~MyLabel()
{
}
// Este método recibe una imagen de tipo QImage y la convierte en un QPixmap para poder mostrarla en el QLabel.
void MyLabel::setImage(QImage image) {
  QPixmap pixmap = QPixmap::fromImage(image); // Convierte el objeto QImage en un objeto
  int w = this->width(); //Obtiene el ancho del widget MyLabel actual
  int h = this->height(); // Obtiene la altura del widget MyLabel actual.
// Escala el pixmap para ajustarse a las dimensiones del widget (w x h) y usa el método setPixmap de QLabel para mostrar la imagen escalada en el MyLabel. 
 setPixmap(pixmap.scaled(w, h, Qt::KeepAspectRatio)); 
}

