#include "IMICam.h"


IMICam::IMICam() {
        g_pCamera = nullptr;
        pDevice = nullptr;
    g_iMode = NEPTUNE_GRAB_ONE;
}

IMICam::~IMICam() {
    //if (Opened) Close();
}

int IMICam::InitLib() {
    // Initialize Neptune class library.
    static bool bInitLib = false;
    if (bInitLib == false) {
        if(::InitLibrary() != NEPTUNE_ERR_Success )
            return 0;
        bInitLib = true;
    }
    // Get number of cameras connected to the system.
    int nCam = CDeviceManager::GetInstance().GetTotalCamera();
    if( nCam == 0 )
        return 0;
    g_pCamInfo = new NEPTUNE_CAM_INFO[nCam];
    // Get cameras list from connected to the system.
    if( CDeviceManager::GetInstance().GetCameraList(g_pCamInfo, nCam ) != NEPTUNE_ERR_Success )
        return 0;
    return nCam;
}

void IMICam::UnInitLib() {

    if( g_pCamera)
        delete g_pCamera;

    if( g_pCamInfo )
        delete g_pCamInfo;
    // Clear Neptune class library.
    ::UninitLibrary();
}

void IMICam::SetMode(ENeptuneGrabMode iMode) {
    this->g_iMode = iMode;
}

bool IMICam::Open(int index) {
    int nCam = InitLib();
    isOpen = false;
    if(nCam == 0 ) {
        AddLog("Error", "Error while initializing library!", "NEPTUNE");
        //UnInitLib();
        return false;
    }

        // Get INeptuneDeivce interface object. (index base)
        pDevice = CDeviceManager::GetInstance().GetDeviceFromIndex(index);
        if(pDevice == nullptr) {
            AddLog("Error", "IMICam::Open", QString("Camera [ ")  + QString(" ] is not discovered in system!"));
            UnInitLib();
            return false;
        }

        try {
            g_pCamera = new CCameraInstance(pDevice);


        }
        catch ( ... ) {
            AddLog("Error", "IMICam::Open", QString("Can not select camera [")  + QString("]!"));
            delete g_pCamera;
            UnInitLib();
            return false;
        }
        AddLog("OK", "IMICam::Open", QString("Selected Camera [") +  QString("]"));
        AddLog("OK", "IMICam::Open", QString(g_pCamera->GetCamera()->GetVendorName()) +QString(" : ")
               + QString(g_pCamera->GetCamera()->GetModelName()) + QString(" S/N: ") + QString(g_pCamera->GetCamera()->GetSerialNumber()));

        // Set the pixel format to a camera.
        if(g_pCamera->SetPixelFormat( Mono8 ) != NEPTUNE_ERR_Success ) {
            if( g_pCamera->GetCameraType() != NEPTUNE_DEV_TYPE_GIGE )
                AddLog("Error", "IMICam::Open", QString("Camera [ ")  + QString(" ] interface is not a GigE!"));
            else
                AddLog("Error", "IMICam::Open", QString("Pixel format setting error!"));
            UnInitLib();
            return false;
        }


        if( g_pCamera->AcquisitionStart(g_iMode) != NEPTUNE_ERR_Success ) {
            AddLog("Error", "IMICam::Open", "Acquisition start error!");
            UnInitLib();
            return false;
        }



    isOpen = true;
    return isOpen;
}

void IMICam::Close() {

    if(g_pCamera->AcquisitionStop() != NEPTUNE_ERR_Success) {
        AddLog("Error", "IMICam::Close", "Acquisition stop error!");
    }

    UnInitLib();
}

void IMICam::Stop() {

    if(g_pCamera->AcquisitionStop() != NEPTUNE_ERR_Success) {
        AddLog("Error", "IMICam::Close", "Acquisition stop error!");

    }
}

bool IMICam::Grab(Mat &grabImg) {
    cv::Mat rawImg;

    if (this->g_iMode == NEPTUNE_GRAB_ONE) {

        if(this->g_pCamera->OneFrameGrab(pData, 100) != NEPTUNE_ERR_Success) {
            AddLog("Error", "IMICam::Grab", QString("Camera ")  + QString(" grab one error!"));
            return false;
        }


        grabImg = cv::Mat((int)pData->GetHeight(), (int)pData->GetWidth(), CV_8UC1, (char *)pData->GetBufferPtr());
        //grabImg = vision.RotateImage(grabImg, DEGREE_90);
    }
    else if (g_iMode == NEPTUNE_GRAB_CONTINUOUS) {
        if( g_pCamera->WaitEventDataStream(pData, 1000) == NEPTUNE_ERR_TimeOut) {
            AddLog("Error", "IMICam::Grab", QString("Camera ")  + QString(" grab one error!"));
            return false;
        }
        cv::Mat( (int)pData->GetHeight(), (int)pData->GetWidth(), CV_8UC1, (char *)pData->GetBufferPtr()).copyTo(grabImg);
        g_pCamera->QueueBufferDataStream( pData->GetBufferIndex() );
    }
    if (grabImg.empty()) return false;
    return true;
}

bool IMICam::IsGrabbing() {
    return true;
}

bool IMICam::IsOpen() {
    return isOpen;
}

bool IMICam::SetROI(Rect rect, int iAngle)
{

    NEPTUNE_IMAGE_SIZE format, max;

    //format.nStartX =0;
   // format.nStartY =0;
    format.nSizeX = rect.width;
    format.nSizeY = rect.height;

    g_pCamera->SetImageSize(format);
    format.nStartX = rect.x;
    format.nStartY =rect.y;
    //format.nSizeX = rect.width;
    //format.nSizeY = rect.height;
    //format.nSizeX = rect.width;
    //format.nSizeY = rect.height;
    //g_pCamera->GetMaxImageSize(&max);

    if(!g_pCamera->SetImageSize(format) != NEPTUNE_ERR_Success){
        AddLog("Error", "IMICam::Open", "Setting fail");
        return false;
    }
    switch (iAngle) {
    case DEGREE_0:
    {
        if(g_pCamera->SetRotate(NEPTUNE_ROTATE_0) != NEPTUNE_ERR_Success)
            return false;
        break;
    }
    case DEGREE_90:
        if(g_pCamera->SetRotate(NEPTUNE_ROTATE_90) != NEPTUNE_ERR_Success)
            return false;
        break;
    case DEGREE_180:
        if(g_pCamera->SetRotate(NEPTUNE_ROTATE_180) != NEPTUNE_ERR_Success)
            return false;
        break;
    case DEGREE_270:
        if(g_pCamera->SetRotate(NEPTUNE_ROTATE_270) != NEPTUNE_ERR_Success)
            return false;
        break;
    default:
        break;
    }

    return true;
}

void IMICam::AddLog(QString arg_log) {
    cerr << arg_log.toStdString() << endl;
}

void IMICam::AddLog(QString arg_category, QString arg_function, QString arg_msg) {
    static long m_iLogIndex = 0;
    QString sTmp;
    sTmp = "[" + QString::number(m_iLogIndex++) + "][" + arg_category
            + "]:[" + arg_function + "]>[" + arg_msg + "]";
    AddLog(sTmp);
}

QString IMICam::GetNameCam()
{
    return QString(this->g_pCamera->GetCamera()->GetSerialNumber());

}
Mat IMICam::rotateImage(Mat inImage, Point center, double angle)
{
    cv::Mat retImage/*(inImage.rows, inImage.cols, CV_8UC3, Scalar(212, 230, 233))*/;
    cv::Mat r = getRotationMatrix2D(center, angle, 1.0);
    cv::warpAffine(inImage, retImage, r, cv::Size(retImage.cols, retImage.rows), INTER_LINEAR, BORDER_CONSTANT, Scalar(212, 230, 233));

    return retImage;
}

void IMICam::GetImageSize(NEPTUNE_IMAGE_SIZE &ImgSize)
{
    g_pCamera->GetImageSize(&ImgSize);
}


