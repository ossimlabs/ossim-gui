//---
//
// License:  See top level LICENSE.txt file.
//
// Description:  Description: Dialog box for chipping/exporting images.
//
//---
// $Id$

#include <ossimGui/ChipperDialog.h>

#include <QApplication>
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <cstring>
#include <iomanip>
#include <sstream>
#include <vector>

ossimGui::ChipperDialog::ChipperDialog(QWidget* parent, Qt::WindowFlags f) 
   :
   QDialog( parent, f ),
   m_outputFileLineEdit(0),
   m_outputFilePushButton(),
   m_outputTypeComboBox(),
   m_editWriterPushButton(),
   m_gsdLineEdit(),
   m_linesLineEdit(),
   m_samplesLineEdit(),
   m_sceneRectPushButton(),
   m_saveSpecFilePushButton(),
   m_saveImagePushButton(),
   m_closePushButton()
{
   // set title in Polygon Remapper Dialog
   this->setWindowTitle( tr("chipper") );
   
   // setup vertical layout   
   QVBoxLayout* vbox0 = new QVBoxLayout();
   
   // Row 1:
   QGroupBox* outputImageGroupBox = new QGroupBox(tr("output image"));   
   QHBoxLayout* hboxR1 = new QHBoxLayout();

   // Line edit:
   m_outputFileLineEdit = new QLineEdit();
   hboxR1->addWidget(m_outputFileLineEdit);

   // File dialog:
   m_outputFilePushButton = new QPushButton( tr("file") );
   hboxR1->addWidget(m_outputFilePushButton);

   // Output type/writer:
   m_outputTypeComboBox = new QComboBox();
   hboxR1->addWidget(m_outputTypeComboBox);

   // Edit writer:
   m_editWriterPushButton = new QPushButton( tr("edit writer") );
   hboxR1->addWidget(m_editWriterPushButton);

   outputImageGroupBox->setLayout(hboxR1);
   
   vbox0->addWidget(outputImageGroupBox);

   // Row 2:

   QHBoxLayout* hboxR2 = new QHBoxLayout();

   // r2_col1 gsd:

   QGroupBox* gsdGroupBox = new QGroupBox( tr("gsd in meters") );
   QHBoxLayout* hbox_r2_c1 = new QHBoxLayout();
   m_gsdLineEdit = new QLineEdit();
   hbox_r2_c1->addWidget(m_gsdLineEdit);
   gsdGroupBox->setLayout(hbox_r2_c1);
   hboxR2->addWidget(gsdGroupBox);
   
   // lines:
   QGroupBox* linesGroupBox = new QGroupBox(tr("lines"));
   QHBoxLayout* hbox_r2_c2 = new QHBoxLayout();
   m_linesLineEdit = new QLineEdit();
   hbox_r2_c2->addWidget(m_linesLineEdit);
   linesGroupBox->setLayout(hbox_r2_c2);
   hboxR2->addWidget(linesGroupBox);

   // samples:
   QGroupBox* samplesGroupBox = new QGroupBox(tr("samples"));
   QHBoxLayout* hbox_r2_c3 = new QHBoxLayout();
   m_samplesLineEdit = new QLineEdit();
   hbox_r2_c3->addWidget(m_samplesLineEdit);
   samplesGroupBox->setLayout(hbox_r2_c3);
   hboxR2->addWidget(samplesGroupBox);

   // Scene rect button:
   m_sceneRectPushButton = new QPushButton( tr("use scene rect") );
   hboxR2->addWidget( m_sceneRectPushButton );

   vbox0->addLayout(hboxR2);

   // Row 3:
   QHBoxLayout* hboxR3 = new QHBoxLayout();
   
   // save spec:
   m_saveSpecFilePushButton = new QPushButton( tr("save spec file") );
   hboxR3->addWidget( m_saveSpecFilePushButton );

   // save image:
   m_saveImagePushButton = new QPushButton( tr("save image") );
   hboxR3->addWidget( m_saveImagePushButton );

   // close:
   m_closePushButton = new QPushButton( tr("close") );
   hboxR3->addWidget( m_closePushButton );

   vbox0->addLayout(hboxR3);

   setLayout(vbox0);
}

void ossimGui::ChipperDialog::mousePress(
   QMouseEvent* /* e */, const ossimDpt& /* scenePt */ )
{
   
} // End: ChipperDialog::mousePress(...)
