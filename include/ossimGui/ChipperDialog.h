//---
//
// License:  See top level LICENSE.txt file.
//
// Description: Dialog box for chipping/exporting images.
//
//---
// $Id$

#ifndef ossimGuiChipperDialog_HEADER

#define ossimGuiChipperDialog_HEADER 1

#include <QDialog>

class ossimDpt;
class QComboBox;
class QLineEdit;
class QMouseEvent;
class QPushButton;
class QTextEdit;

namespace ossimGui
{
   class ChipperDialog : public QDialog
   {
   Q_OBJECT

   public:
      
      /** @brief default constructor */
      ChipperDialog( QWidget* parent=0, Qt::WindowFlags f = 0 );

   public slots:

      void mousePress(QMouseEvent* e, const ossimDpt& scenePoint);
      
   protected:

   private:

      QLineEdit*    m_outputFileLineEdit;
      QPushButton*  m_outputFilePushButton;
      QComboBox*    m_outputTypeComboBox;
      QPushButton*  m_editWriterPushButton;
      QLineEdit*    m_gsdLineEdit;
      QLineEdit*    m_linesLineEdit;
      QLineEdit*    m_samplesLineEdit;
      QPushButton*  m_sceneRectPushButton;
      QPushButton*  m_saveSpecFilePushButton;
      QPushButton*  m_saveImagePushButton;
      QPushButton*  m_closePushButton;
   };
}

#endif /* #ifndef ossimGuiChipperDialog_HEADER */
