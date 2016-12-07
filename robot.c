#include <stdio.h>
#include <ctype.h>
#include <cv.h>
#include <highgui.h>
#include <curses.h>
#include "get_contour.h"
#include <math.h>
#define PORT "/dev/ttyACM0" //適宜変更のこと
#include "serial2016.h"

#define CAMERA_CENTER_H 90 //カメラサーボの垂直方向中央値（キャリブレーションに利用）
#define CAMERA_CENTER_V 90 //カメラサーボの垂直方向中央値（キャリブレーションに利用）
#define MOTOR_DEFAULT_L 121 //左モータのデフォルト値（キャリブレーションに利用）
#define MOTOR_DEFAULT_R 136 //右モータのデフォルト値（キャリブレーションに利用）
#define CAMERA_INIT_V 65 //カメラサーボの垂直方向初期値
#define CAMERA_INIT_H 96 //カメラサーボの水平方向初期値
#define CAMERA_FAR 80 //カメラサーボの垂直方向。遠くを探すとき
#define CAMERA_NEAR 50 //カメラサーボの垂直方向。近くを探すとき
#define FIRSTMOVE 30 //最初に動く距離
#define AREA1 400 //最初に赤紙発見に使う面積
#define SECONDMOVE 80 //赤紙を発見してから動く距離
#define DISTANCEFROMTARGET 40 //⑤での的とロボとの間合い
void on_mouse(int event, int x, int y, int flags, void *param);
void r_spin(double angle);
void l_spin(double angle);
void herer_spin(double angle);
void herel_spin(double angle);
void move(int dist);
double angle_convert(int h,int w,double angle);
int main(int argc, char **argv)
{
  CvCapture *capture = NULL;
  IplImage *frame;     // キャプチャ画像 (RGB)
  IplImage* frameHSV;  // キャプチャ画像 (HSV)
  IplImage* mask;      // 指定値によるmask (１チャネル)
  IplImage* contour;   // GetLargestContour() の結果
  IplImage** frames[] = {&frame, &frameHSV};
  IplImage *frameGray;     // キャプチャ画像 (RGB)
  CvMemStorage *storage;
   float *p;
  CvSeq *circles = NULL;
  contourInfo topContoursInfo[CONTOURS];
  int key;
  int stop=0;
  int i=0;
  int s=100;
  init();
  int find=0;
  int de_speed=0;
  CvBox2D oblique = topContoursInfo[0].oblique; // 認識した物体を囲む長方形      
  int x = oblique.center.x;                            // 認識した物体の画面内のx座標(0~319)
  int y = oblique.center.y;                            // 認識した物体の画面内のy座標(0~239)
  //int step1=1;
  int findCircle=0;
  int step2=1;
  int step3=0;
  int step4=0;
  int step5=0;
  int step6=0;
  int step7=0;
  int left=0;
  int right=0;
  int is_hough=0;
  int dis;
  int target=0;
  double theta;
  int v=CAMERA_INIT_V;
  motor_on(MOTOR_DEFAULT_L, MOTOR_DEFAULT_R); // モーター静止パルス幅のキャリブレーション
  camera_on(CAMERA_CENTER_H,CAMERA_CENTER_V);    // カメラアングルキャリブレーション

  camera_horizontal(CAMERA_INIT_H); // 水平方向のカメラ角度を初期値に
  camera_vertical(CAMERA_INIT_V); // 垂直方向のカメラ角度を初期値に

  // 赤系のHSV色．各自チューニングすること
  uchar minH = 110, maxH = 130;
  uchar minS = 150, maxS = 190;
  uchar minV =  80, maxV = 160;

  if (argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
    capture = cvCaptureFromCAM(argc == 2 ? argv[1][0] - '0' : -1);
  if (capture == NULL) {
    printf("not find camera\n");
    return -1;
  }
  // 解析速度向上のために画像サイズを下げる
  cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, 320);
  cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, 240);
  frame = cvQueryFrame(capture);
  frameHSV = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
  mask = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
  contour = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
  cvNamedWindow("src", CV_WINDOW_AUTOSIZE);
  cvNamedWindow("contour", CV_WINDOW_AUTOSIZE);
  cvSetMouseCallback("src", on_mouse, (void *)frames);
  cvSetMouseCallback("contour", on_mouse, (void *)frames);
 motor(128, 128);
  move(FIRSTMOVE);//①スタート位置からの前進
  while (1) {
    frame = cvQueryFrame(capture);
    cvCvtColor(frame, frameHSV, CV_RGB2HSV);
    cvShowImage("src", frame);
    GetMaskHSV(frame, mask, minH, maxH, minS, maxS, minV, maxV);
    GetLargestContour(frame, mask, contour, topContoursInfo);
    cvShowImage("contour", contour);
    key = cvWaitKey(1);
if(target==1){ //緑を見つける パラメーターの設定
minH = 40, maxH = 60;
minS = 50, maxS = 150; 
minV = 100, maxV = 220;}
if(target==2){ //青を見つける パラメーターの設定
minH = 10,maxH = 25;
minS = 120, maxS = 210; 
minV = 125, maxV = 165;}
    if(step2){ //②回転かつ探索
 motor(128, 160); // 見つける
      if(topContoursInfo[0].obliqueArea>AREA1){
        camera_vertical(CAMERA_INIT_V);
        step2=0;
     	step3=1;
        motor_stop();
        move(10);
	  }
    }
   else if(step3){//③全体像が見えるまで対象に近づく。
          move(50);
          step3=0;
     	  step4=1;
    }
    else if(step4){//④回転して軸合わせ 
  	  motor(128,150);
      if(1400<topContoursInfo[0].area){// 一定以上の面積を感知出来なければ無ければ軸合わせをしない    
      	motor_stop();  
theta=angle_convert(topContoursInfo[0].oblique.size.height,topContoursInfo[0].oblique.size.width,topContoursInfo[0].oblique.angle);  
        x = topContoursInfo[0].oblique.center.x;
        y = topContoursInfo[0].oblique.center.y;
        printf("(x=%d y=%d theta=%lf \n)",x,y,theta);
        if(124-x>0)
        dis=(257-y)*sin(3.14*theta/180)+(abs(124-x))*cos(3.14*theta/180)-30;
        else
        dis=(257-y)*sin(3.14*theta/180)+(abs(124-x))*cos(3.14*theta/180)+10;
        usleep(1000000);
        herel_spin(60-angle_convert(topContoursInfo[0].oblique.size.height,topContoursInfo[0].oblique.size.width,topContoursInfo[0].oblique.angle));
        usleep(300000);
        step4=0;
      	step5=1;
      
      }
    } 
    else if(step5){ //⑤垂直状態から的の正面に持っていく
  	  x = topContoursInfo[0].oblique.center.x;
      move(dis); //調整が必要
      herer_spin(90);
      y = topContoursInfo[0].oblique.center.y;//ターゲットを補足
      move(-20); //調整が必要     	
  	  step5=0;
  	  step6=1;	
 
    }
    else if(step6){ //⑥ハフ変換で位置を調整して的を倒す
	int cx = 0;
	int cy = 0;
        frame = cvQueryFrame(capture);
        frameGray = cvCreateImage (cvGetSize(frame), IPL_DEPTH_8U, 1);
	// (1) カメラからの入力画像1フレームをframeに格納
    	frame = cvQueryFrame(capture);
    	cvCvtColor(frame, frameGray, CV_RGB2GRAY);
        // (2)ハフ変換のための前処理（画像を平滑化しないと誤検出が出やすい）
    	cvSmooth (frameGray, frameGray, CV_GAUSSIAN, 11, 0, 0, 0);
    	storage = cvCreateMemStorage (0);
    	// (3)ハフ変換による円の検出と検出した円の描画
    	circles = cvHoughCircles (frameGray, storage, CV_HOUGH_GRADIENT, 
                              1, 3.0, 20.0, 70.0, 10,
                              	MAX (frameGray->width, frameGray->height));
    	for (i = 0; i < MIN (3,circles->total); i++) {
      	p = (float *) cvGetSeqElem (circles, i);
	cx = p[0] + cx;
        cy=  p[1] + cy;
      	}
        if(cx!=0){ 
        find=1; //的を見つけた
  	cx = cx / MIN(3,circles->total);} //average x
  	printf("中心=%d\n",cx);
        if(cx==0&&cy<60){
          printf("up\n");
          
        }

        if(cx!=0&&cx<60) {
  		//左回転(90)
                l_spin(3);}
  			//move(視界の中心-cx);
  		// 右回転
	else if(cx>80){	
           r_spin(3);
          }
  	else {move(3);findCircle++;}		

	else if(cy>200*3){
          move(40);
          target++;
           findCircle=0;
           move(-190);
           herel_spin(90);
           move(10);
           step2=1;
  	   step6=0;}
}
    else
      {stop=0;
      /// 赤い物体が見つからなかった場合
    }
    if (key == 'q') break;
  }
  finalize();
  cvDestroyWindow("src");
  cvDestroyWindow("contour");
  cvReleaseImage(&frameHSV);
  cvReleaseImage(&mask);
  cvReleaseImage(&contour);
  cvReleaseCapture(&capture);
  return 0;
}
void on_mouse(int event, int x, int y, int flags, void *frames)
{
  CvScalar BGR, HSV;
  if (event == CV_EVENT_MOUSEMOVE) {
      BGR = cvGet2D(*(((IplImage***)frames)[0]), y, x);
      HSV = cvGet2D(*(((IplImage***)frames)[1]), y, x);
      printf("(%3d,%3d): RGB=(%3.0f,%3.0f,%3.0f) HSV=(%3.0f,%3.0f,%3.0f)\n",
             x, y, BGR.val[2], BGR.val[1], BGR.val[0],
             HSV.val[0], HSV.val[1], HSV.val[2]);
    }
}
void r_spin(double angle){
	motor(160,125);
	usleep(1*1000000/(360/7)*angle);
	motor_stop();
}
void l_spin(double angle){
	motor(125,160);
	usleep(1*1000000/(360/5.95)*angle);
	motor_stop();
}
void herer_spin(double angle){
	motor(160,125);
	usleep(1*1000000/(360/7)*angle);
	motor_stop();
}
void herel_spin(double angle){
	motor(125,160);
	usleep(1*1000000/(360/5.95)*angle);
	motor_stop();
}
void move(int dist){
        printf("inMove %d\n",dist);
	if(dist>0){
	motor(140,140);
	usleep(dist*1000000*2.72/88);
	motor_stop();}
       else if(dist<0){
	motor(116,116);
	usleep(-dist*1000000*2.72/88);
	motor_stop();}
}
double angle_convert(int h,int w,double angle){ 
  if(w>h){
   return -angle;
  }
  else{
    return 90+angle;

  }
}
