#include <ossimGui/About.h>
#include <QtGui/QPixmap>
ossimGui::About::About(QWidget* parent)
:QDialog(parent)
{
   setupUi(this);
   m_logo->setPixmap(QPixmap(":/logos/RadiantBlue2.png"));
   setAttribute(Qt::WA_DeleteOnClose);
   connect(m_okButton, SIGNAL(clicked(bool)), this, SLOT(close()));
}