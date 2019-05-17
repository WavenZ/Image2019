#define _CRT_SECURE_NO_WARNINGS
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <cctype>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>

using namespace cv;
using namespace std;

const char * usage =
" \nexample command line for calibration from a live feed.\n"
"   calibration  -w=4 -h=5 -s=0.025 -o=camera.yml -op -oe\n"
" \n"
" example command line for calibration from a list of stored images:\n"
"   imagelist_creator image_list.xml *.png\n"
"   calibration -w=4 -h=5 -s=0.025 -o=camera.yml -op -oe image_list.xml\n"
" where image_list.xml is the standard OpenCV XML/YAML\n"
" use imagelist_creator to create the xml or yaml list\n"
" file consisting of the list of strings, e.g.:\n"
" \n"
"<?xml version=\"1.0\"?>\n"
"<opencv_storage>\n"
"<images>\n"
"view000.png\n"
"view001.png\n"
"<!-- view002.png -->\n"
"view003.png\n"
"view010.png\n"
"one_extra_view.jpg\n"
"</images>\n"
"</opencv_storage>\n";




const char* liveCaptureHelp =
    "When the live video from camera is used as input, the following hot-keys may be used:\n"
        "  <ESC>, 'q' - quit the program\n"
        "  'g' - start capturing images\n"
        "  'u' - switch undistortion on/off\n";

static void help()
{
    printf( "This is a camera calibration sample.\n"
        "Usage: calibration\n"
        "     -w=<board_width>         # the number of inner corners per one of board dimension\n"
        "     -h=<board_height>        # the number of inner corners per another board dimension\n"
        "     [-pt=<pattern>]          # the type of pattern: chessboard or circles' grid\n"
        "     [-n=<number_of_frames>]  # the number of frames to use for calibration\n"
        "                              # (if not specified, it will be set to the number\n"
        "                              #  of board views actually available)\n"
        "     [-d=<delay>]             # a minimum delay in ms between subsequent attempts to capture a next view\n"
        "                              # (used only for video capturing)\n"
        "     [-s=<squareSize>]       # square size in some user-defined units (1 by default)\n"
        "     [-o=<out_camera_params>] # the output filename for intrinsic [and extrinsic] parameters\n"
        "     [-op]                    # write detected feature points\n"
        "     [-oe]                    # write extrinsic parameters\n"
        "     [-oo]                    # write refined 3D object points\n"
        "     [-zt]                    # assume zero tangential distortion\n"
        "     [-a=<aspectRatio>]      # fix aspect ratio (fx/fy)\n"
        "     [-p]                     # fix the principal point at the center\n"
        "     [-v]                     # flip the captured images around the horizontal axis\n"
        "     [-V]                     # use a video file, and not an image list, uses\n"
        "                              # [input_data] string for the video file name\n"
        "     [-su]                    # show undistorted images after calibration\n"
        "     [-ws=<number_of_pixel>]  # Half of search window for cornerSubPix (11 by default)\n"
        "     [-dt=<distance>]         # actual distance between top-left and top-right corners of\n"
        "                              # the calibration grid. If this parameter is specified, a more\n"
        "                              # accurate calibration method will be used which may be better\n"
        "                              # with inaccurate, roughly planar target.\n"
        "     [input_data]             # input data, one of the following:\n"
        "                              #  - text file with a list of the images of the board\n"
        "                              #    the text file can be generated with imagelist_creator\n"
        "                              #  - name of video file with a video of the board\n"
        "                              # if input_data not specified, a live view from the camera is used\n"
        "\n" );
    printf("\n%s",usage);
    printf( "\n%s", liveCaptureHelp );
}

enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };
enum Pattern { CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };

static double computeReprojectionErrors(
        const vector<vector<Point3f> >& objectPoints,
        const vector<vector<Point2f> >& imagePoints,
        const vector<Mat>& rvecs, const vector<Mat>& tvecs,
        const Mat& cameraMatrix, const Mat& distCoeffs,
        vector<float>& perViewErrors )
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for( i = 0; i < (int)objectPoints.size(); i++ )
    {
        projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i],
                      cameraMatrix, distCoeffs, imagePoints2);
        err = norm(Mat(imagePoints[i]), Mat(imagePoints2), NORM_L2);
        int n = (int)objectPoints[i].size();
        perViewErrors[i] = (float)std::sqrt(err*err/n);
        totalErr += err*err;
        totalPoints += n;
    }

    return std::sqrt(totalErr/totalPoints);
}

static void calcChessboardCorners(Size boardSize, float squareSize, vector<Point3f>& corners, Pattern patternType = CHESSBOARD)
{
    corners.resize(0);

    switch(patternType)
    {
      case CHESSBOARD:
      case CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
            for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float(j*squareSize),
                                          float(i*squareSize), 0));
        break;

      case ASYMMETRIC_CIRCLES_GRID:
        for( int i = 0; i < boardSize.height; i++ )
            for( int j = 0; j < boardSize.width; j++ )
                corners.push_back(Point3f(float((2*j + i % 2)*squareSize),
                                          float(i*squareSize), 0));
        break;

      default:
        CV_Error(Error::StsBadArg, "Unknown pattern type\n");
    }
}

static bool runCalibration( vector<vector<Point2f> > imagePoints,
                    Size imageSize, Size boardSize, Pattern patternType,
                    float squareSize, float aspectRatio,
                    float grid_width, bool release_object,
                    int flags, Mat& cameraMatrix, Mat& distCoeffs,
                    vector<Mat>& rvecs, vector<Mat>& tvecs,
                    vector<float>& reprojErrs,
                    vector<Point3f>& newObjPoints,
                    double& totalAvgErr)
{
    cameraMatrix = Mat::eye(3, 3, CV_64F);
    if( flags & CALIB_FIX_ASPECT_RATIO )
        cameraMatrix.at<double>(0,0) = aspectRatio;

    distCoeffs = Mat::zeros(8, 1, CV_64F);

    vector<vector<Point3f> > objectPoints(1);
    calcChessboardCorners(boardSize, squareSize, objectPoints[0], patternType);
    objectPoints[0][boardSize.width - 1].x = objectPoints[0][0].x + grid_width;
    newObjPoints = objectPoints[0];

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    double rms;
    int iFixedPoint = -1;
    if (release_object)
        iFixedPoint = boardSize.width - 1;
    rms = calibrateCameraRO(objectPoints, imagePoints, imageSize, iFixedPoint,
                            cameraMatrix, distCoeffs, rvecs, tvecs, newObjPoints,
                            flags | CALIB_FIX_K3 | CALIB_USE_LU);
    printf("RMS error reported by calibrateCamera: %g\n", rms);

    bool ok = checkRange(cameraMatrix) && checkRange(distCoeffs);

    if (release_object) {
        cout << "New board corners: " << endl;
        cout << newObjPoints[0] << endl;
        cout << newObjPoints[boardSize.width - 1] << endl;
        cout << newObjPoints[boardSize.width * (boardSize.height - 1)] << endl;
        cout << newObjPoints.back() << endl;
    }

    objectPoints.clear();
    objectPoints.resize(imagePoints.size(), newObjPoints);
    totalAvgErr = computeReprojectionErrors(objectPoints, imagePoints,
                rvecs, tvecs, cameraMatrix, distCoeffs, reprojErrs);

    return ok;
}


static void saveCameraParams( const string& filename,
                       Size imageSize, Size boardSize,
                       float squareSize, float aspectRatio, int flags,
                       const Mat& cameraMatrix, const Mat& distCoeffs,
                       const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                       const vector<float>& reprojErrs,
                       const vector<vector<Point2f> >& imagePoints,
                       const vector<Point3f>& newObjPoints,
                       double totalAvgErr )
{
    FileStorage fs( filename, FileStorage::WRITE );

    time_t tt;
    time( &tt );
    struct tm *t2 = localtime( &tt );
    char buf[1024];
    strftime( buf, sizeof(buf)-1, "%c", t2 );

    fs << "calibration_time" << buf;

    if( !rvecs.empty() || !reprojErrs.empty() )
        fs << "nframes" << (int)std::max(rvecs.size(), reprojErrs.size());
    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs << "board_width" << boardSize.width;
    fs << "board_height" << boardSize.height;
    fs << "square_size" << squareSize;

    if( flags & CALIB_FIX_ASPECT_RATIO )
        fs << "aspectRatio" << aspectRatio;

    if( flags != 0 )
    {
        sprintf( buf, "flags: %s%s%s%s",
            flags & CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
            flags & CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
            flags & CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
            flags & CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "" );
        //cvWriteComment( *fs, buf, 0 );
    }

    fs << "flags" << flags;

    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;

    fs << "avg_reprojection_error" << totalAvgErr;
    if( !reprojErrs.empty() )
        fs << "per_view_reprojection_errors" << Mat(reprojErrs);

    if( !rvecs.empty() && !tvecs.empty() )
    {
        CV_Assert(rvecs[0].type() == tvecs[0].type());
        Mat bigmat((int)rvecs.size(), 6, rvecs[0].type());
        for( int i = 0; i < (int)rvecs.size(); i++ )
        {
            Mat r = bigmat(Range(i, i+1), Range(0,3));
            Mat t = bigmat(Range(i, i+1), Range(3,6));

            CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
            CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
            //*.t() is MatExpr (not Mat) so we can use assignment operator
            r = rvecs[i].t();
            t = tvecs[i].t();
        }
        //cvWriteComment( *fs, "a set of 6-tuples (rotation vector + translation vector) for each view", 0 );
        fs << "extrinsic_parameters" << bigmat;
    }

    if( !imagePoints.empty() )
    {
        Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
        for( int i = 0; i < (int)imagePoints.size(); i++ )
        {
            Mat r = imagePtMat.row(i).reshape(2, imagePtMat.cols);
            Mat imgpti(imagePoints[i]);
            imgpti.copyTo(r);
        }
        fs << "image_points" << imagePtMat;
    }

    if( !newObjPoints.empty() )
    {
        fs << "grid_points" << newObjPoints;
    }
}

static bool readStringList( const string& filename, vector<string>& l )
{
    l.resize(0);
    FileStorage fs(filename, FileStorage::READ);
    if( !fs.isOpened() )
        return false;
    size_t dir_pos = filename.rfind('/');
    if (dir_pos == string::npos)
        dir_pos = filename.rfind('\\');
    FileNode n = fs.getFirstTopLevelNode();
    if( n.type() != FileNode::SEQ )
        return false;
    FileNodeIterator it = n.begin(), it_end = n.end();
    for( ; it != it_end; ++it )
    {
        string fname = (string)*it;
        if (dir_pos != string::npos)
        {
            string fpath = samples::findFile(filename.substr(0, dir_pos + 1) + fname, false);
            if (fpath.empty())
            {
                fpath = samples::findFile(fname);
            }
            fname = fpath;
        }
        else
        {
            fname = samples::findFile(fname);
        }
        l.push_back(fname);
    }
    return true;
}


static bool runAndSave(const string& outputFilename,
                const vector<vector<Point2f> >& imagePoints,
                Size imageSize, Size boardSize, Pattern patternType, float squareSize,
                float grid_width, bool release_object,
                float aspectRatio, int flags, Mat& cameraMatrix,
                Mat& distCoeffs, bool writeExtrinsics, bool writePoints, bool writeGrid )
{
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;
    vector<Point3f> newObjPoints;

    bool ok = runCalibration(imagePoints, imageSize, boardSize, patternType, squareSize,
                   aspectRatio, grid_width, release_object, flags, cameraMatrix, distCoeffs,
                   rvecs, tvecs, reprojErrs, newObjPoints, totalAvgErr);
    printf("%s. avg reprojection error = %.7f\n",
           ok ? "Calibration succeeded" : "Calibration failed",
           totalAvgErr);

    if( ok )
        saveCameraParams( outputFilename, imageSize,
                         boardSize, squareSize, aspectRatio,
                         flags, cameraMatrix, distCoeffs,
                         writeExtrinsics ? rvecs : vector<Mat>(),
                         writeExtrinsics ? tvecs : vector<Mat>(),
                         writeExtrinsics ? reprojErrs : vector<float>(),
                         writePoints ? imagePoints : vector<vector<Point2f> >(),
                         writeGrid ? newObjPoints : vector<Point3f>(),
                         totalAvgErr );
    return ok;
}

/************************************************
	@brief: Monocular camera calibration
	
	@author: WavenZ

	@time: 2019/4/29
*************************************************/
int main( int argc, char** argv )
{
	/*************************1.一堆参数***************************/
	Size boardSize = { 12, 6 }; // 标定靶的尺寸
	Size imageSize; // 图像尺寸

	float squareSize = 25.0f; // 方格尺寸
	float aspectRatio = 1; // 宽长比
	float grid_width = squareSize * (boardSize.width - 1); // 方格总宽度(不算最外一圈方格)


    string outputFilename = "out_camera_data.yml"; // 输出文件名
    string inputFilename = "imagelist.yaml"; // 输入文件名


	bool writeExtrinsics = true; // 是否保存外参
	bool writePoints = true; // 是否保存角点
    bool undistortImage = true; // 显示有/无畸变图像
	bool writeGrid = false; // 是否保存网格
	bool release_object = false;


	int flags = 0;
	int winSize = 11; // 亚像素级角点检测窗口大小
	int nframes; // 图像张数

    vector<vector<Point2f> > imagePoints; // 用于保存所有图的角点
    vector<string> imageList; // 图像列表，从imagelist.yaml中读入
    Pattern pattern = CHESSBOARD; // 靶标图案（棋盘）

	Mat cameraMatrix, distCoeffs; // 摄像机矩阵和畸变系数(就是我们要得到的标定参数)
	/*************************1.一堆参数***************************/

	/*************************2.标定过程***************************/
	readStringList(samples::findFile(inputFilename), imageList); // 从imagelist.yaml文件中读取图像列表

    nframes = (int)imageList.size(); // 图像帧数

    namedWindow( "Image View", 1 ); // 新建窗口

	// 对所有的图进行角点检测
	for (int i = 0; i < nframes; i++) {
		Mat view = imread(imageList[i], 1);
		imageSize = view.size(); // 图像尺寸
		vector<Point2f> pointbuf; // 角点缓存

		// 寻找角点
		bool found = findChessboardCorners(view, boardSize, pointbuf,
			CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK | CALIB_CB_NORMALIZE_IMAGE);
		if (found) {
			Mat viewGray;
			cvtColor(view, viewGray, COLOR_BGR2GRAY); // 转灰度图
			// 亚像素角点检测提升精度
			cornerSubPix(viewGray, pointbuf, Size(winSize, winSize),
				Size(-1, -1), TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 30, 0.0001));
			imagePoints.push_back(pointbuf); // 将找到的角点进行存储
			drawChessboardCorners(view, boardSize, Mat(pointbuf), found); //画出角点
		}
		imshow("Image View", view); // 显示
		waitKey(10);

	}
	if (imagePoints.size() > 0) // 找到的角点组数大于0，进行标定并保存标定参数到out_camera_data.yml
		runAndSave(outputFilename, imagePoints, imageSize,
			boardSize, pattern, squareSize, grid_width, release_object, aspectRatio,
			flags, cameraMatrix, distCoeffs,
			writeExtrinsics, writePoints, writeGrid);
	/*************************2.标定过程***************************/
	destroyAllWindows();
	return 0;
}
