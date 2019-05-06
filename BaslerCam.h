/* BASLERCAM LIBRARY */
// Library used: C++ Standard Library, OpenCV, Pylon C++
// Support: MSVC compiler, GCC compiler with IDE Qt
// Edit: Hay Cuoi
// Date: 2018/12/12


#ifndef BASLERCAM_H
#define	BASLERCAM_H

#include <QWidget>
#include <QString>
#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <opencv2/opencv.hpp>
#include <QMutex>
#include "myDefine.h"

using namespace cv;
using namespace std;
using namespace Pylon;

typedef CBaslerGigEInstantCamera Camera_t;

enum eMODE
{
  MODE_SOFTTRIG,
  MODE_HARDTRIG,
  MODE_CONT
};

class BaslerCam : public Pylon::CImageEventHandler, public Pylon::CConfigurationEventHandler
{
public:
  explicit BaslerCam(QString nameCam, int indexCam, int iType);
  ~BaslerCam();
   //QMutex m_CameraCs;

public:
  bool Open();
  bool isOpen();
  void Close();

  bool Grab(cv::Mat & grabImg);
  void Stop();

  bool IsGrabbing();

  void SetExpose(int64_t iValue);
  void SetROI(int x, int y, int width, int height);

  void AddLog(QString arg_log);
  void AddLog(QString arg_category, QString arg_function, QString arg_msg);


private:
  Camera_t *m_pCam;
  eMODE m_iMode;
  QString nameCam;
  int indexCam;
  int typeCam;
 bool isOpened;
};

#endif
