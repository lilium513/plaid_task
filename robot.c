#include <stdio.h>
#include <ctype.h>
#include <cv.h>
#include <highgui.h>
#include <curses.h>
#include "get_contour.h"
#include <math.h>
#define PORT "/dev/ttyACM0" //�K�X�ύX�̂���
#include "serial2016.h"

#define CAMERA_CENTER_H 90 //�J�����T�[�{�̐������������l�i�L�����u���[�V�����ɗ��p�j
#define CAMERA_CENTER_V 90 //�J�����T�[�{�̐������������l�i�L�����u���[�V�����ɗ��p�j
#define MOTOR_DEFAULT_L 121 //�����[�^�̃f�t�H���g�l�i�L�����u���[�V�����ɗ��p�j
#define MOTOR_DEFAULT_R 136 //�E���[�^�̃f�t�H���g�l�i�L�����u���[�V�����ɗ��p�j
#define CAMERA_INIT_V 65 //�J�����T�[�{�̐������������l
#define CAMERA_INIT_H 96 //�J�����T�[�{�̐������������l
#define CAMERA_FAR 80 //�J�����T�[�{�̐��������B������T���Ƃ�
#define CAMERA_NEAR 50 //�J�����T�[�{�̐��������B�߂���T���Ƃ�
#define FIRSTMOVE 30 //�ŏ��ɓ�������
#define AREA1 400 //�ŏ��ɐԎ������Ɏg���ʐ�
#define SECONDMOVE 80 //�Ԏ��𔭌����Ă��瓮������
#define DISTANCEFROMTARGET 40 //�D�ł̓I�ƃ��{�Ƃ̊ԍ���
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
  IplImage *frame;     // �L���v�`���摜 (RGB)
  IplImage* frameHSV;  // �L���v�`���摜 (HSV)
  IplImage* mask;      // �w��l�ɂ��mask (�P�`���l��)
  IplImage* contour;   // GetLargestContour() �̌���
  IplImage** frames[] = {&frame, &frameHSV};
  IplImage *frameGray;     // �L���v�`���摜 (RGB)
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
  CvBox2D oblique = topContoursInfo[0].oblique; // �F���������̂��͂ޒ����`      
  int x = oblique.center.x;                            // �F���������̂̉�ʓ���x���W(0~319)
  int y = oblique.center.y;                            // �F���������̂̉�ʓ���y���W(0~239)
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
  motor_on(MOTOR_DEFAULT_L, MOTOR_DEFAULT_R); // ���[�^�[�Î~�p���X���̃L�����u���[�V����
  camera_on(CAMERA_CENTER_H,CAMERA_CENTER_V);    // �J�����A���O���L�����u���[�V����

  camera_horizontal(CAMERA_INIT_H); // ���������̃J�����p�x�������l��
  camera_vertical(CAMERA_INIT_V); // ���������̃J�����p�x�������l��

  // �Ԍn��HSV�F�D�e���`���[�j���O���邱��
  uchar minH = 110, maxH = 130;
  uchar minS = 150, maxS = 190;
  uchar minV =  80, maxV = 160;

  if (argc == 1 || (argc == 2 && strlen(argv[1]) == 1 && isdigit(argv[1][0])))
    capture = cvCaptureFromCAM(argc == 2 ? argv[1][0] - '0' : -1);
  if (capture == NULL) {
    printf("not find camera\n");
    return -1;
  }
  // ��͑��x����̂��߂ɉ摜�T�C�Y��������
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
  move(FIRSTMOVE);//�@�X�^�[�g�ʒu����̑O�i
  while (1) {
    frame = cvQueryFrame(capture);
    cvCvtColor(frame, frameHSV, CV_RGB2HSV);
    cvShowImage("src", frame);
    GetMaskHSV(frame, mask, minH, maxH, minS, maxS, minV, maxV);
    GetLargestContour(frame, mask, contour, topContoursInfo);
    cvShowImage("contour", contour);
    key = cvWaitKey(1);
if(target==1){ //�΂������� �p�����[�^�[�̐ݒ�
minH = 40, maxH = 60;
minS = 50, maxS = 150; 
minV = 100, maxV = 220;}
if(target==2){ //�������� �p�����[�^�[�̐ݒ�
minH = 10,maxH = 25;
minS = 120, maxS = 210; 
minV = 125, maxV = 165;}
    if(step2){ //�A��]���T��
 motor(128, 160); // ������
      if(topContoursInfo[0].obliqueArea>AREA1){
        camera_vertical(CAMERA_INIT_V);
        step2=0;
     	step3=1;
        motor_stop();
        move(10);
	  }
    }
   else if(step3){//�B�S�̑���������܂őΏۂɋ߂Â��B
          move(50);
          step3=0;
     	  step4=1;
    }
    else if(step4){//�C��]���Ď����킹 
  	  motor(128,150);
      if(1400<topContoursInfo[0].area){// ���ȏ�̖ʐς����m�o���Ȃ���Ζ�����Ύ����킹�����Ȃ�    
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
    else if(step5){ //�D������Ԃ���I�̐��ʂɎ����Ă���
  	  x = topContoursInfo[0].oblique.center.x;
      move(dis); //�������K�v
      herer_spin(90);
      y = topContoursInfo[0].oblique.center.y;//�^�[�Q�b�g��⑫
      move(-20); //�������K�v     	
  	  step5=0;
  	  step6=1;	
 
    }
    else if(step6){ //�E�n�t�ϊ��ňʒu�𒲐����ēI��|��
	int cx = 0;
	int cy = 0;
        frame = cvQueryFrame(capture);
        frameGray = cvCreateImage (cvGetSize(frame), IPL_DEPTH_8U, 1);
	// (1) �J��������̓��͉摜1�t���[����frame�Ɋi�[
    	frame = cvQueryFrame(capture);
    	cvCvtColor(frame, frameGray, CV_RGB2GRAY);
        // (2)�n�t�ϊ��̂��߂̑O�����i�摜�𕽊������Ȃ��ƌ댟�o���o�₷���j
    	cvSmooth (frameGray, frameGray, CV_GAUSSIAN, 11, 0, 0, 0);
    	storage = cvCreateMemStorage (0);
    	// (3)�n�t�ϊ��ɂ��~�̌��o�ƌ��o�����~�̕`��
    	circles = cvHoughCircles (frameGray, storage, CV_HOUGH_GRADIENT, 
                              1, 3.0, 20.0, 70.0, 10,
                              	MAX (frameGray->width, frameGray->height));
    	for (i = 0; i < MIN (3,circles->total); i++) {
      	p = (float *) cvGetSeqElem (circles, i);
	cx = p[0] + cx;
        cy=  p[1] + cy;
      	}
        if(cx!=0){ 
        find=1; //�I��������
  	cx = cx / MIN(3,circles->total);} //average x
  	printf("���S=%d\n",cx);
        if(cx==0&&cy<60){
          printf("up\n");
          
        }

        if(cx!=0&&cx<60) {
  		//����](90)
                l_spin(3);}
  			//move(���E�̒��S-cx);
  		// �E��]
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
      /// �Ԃ����̂�������Ȃ������ꍇ
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
