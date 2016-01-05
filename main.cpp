#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <fstream>



using namespace std;
using namespace cv;
ofstream myfile;


const static int SENSITIVITY_VALUE = 25; // increasing the value removes noise
const static int BLUR_SIZE = 10; //ncrease the size of the object

string intToString(int number){
	//number input and string output
	std::stringstream ss;
	ss << number;
	return ss.str();
}

int main( int argc, char **argv )
{
   VideoCapture capture("Street_main2.mp4"); // open the video file for reading
    ///myfile.open("Result.txt");

        double fps_read = capture.get(CV_CAP_PROP_FPS); //get the frames per seconds of the video
        double frame_count = capture.get(CV_CAP_PROP_FRAME_COUNT);
        cout << "Frame per seconds:"<<fps_read<<endl;
        myfile << "Frame per seconds:"<<fps_read<<endl;
        cout << "\nTotal frames:"<<frame_count<<endl;
        myfile << "\nTotal frames:"<<frame_count<<endl;

        bool pause = false;
        bool objdetectEnabled = false;
        bool trackingEnabled = false;
        bool coordinateEnabled = false;
        bool hogEnabled = false;
        bool debugMode = false;
        //bool allEnabled = false;
        bool allEnabled = true;
        bool allDisabled = false;
        bool x = false;

        VideoWriter writer;

        HOGDescriptor hog;
        hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());

        vector< vector<Point> > contours;


    string filename = "Result_video_f.avi";
    int fcc = CV_FOURCC('D','I','V','3');  //fourcc int
    int fps = 5;
    cv::Size frameSize(capture.get(CV_CAP_PROP_FRAME_WIDTH),capture.get(CV_CAP_PROP_FRAME_HEIGHT));
    writer = VideoWriter(filename,fcc,fps,frameSize);

    if(!writer.isOpened())
    {
        cout <<"ERROR OPENING FILE FOR WRITE"<<endl;
        getchar();
        return -1;
    }

    int c = 0;

    while(1)
    {

        capture.open("Street_Main2.mp4");
		if(!capture.isOpened()){
			cout<<"ERROR ACQUIRING VIDEO FEED\n";
			getchar();
			return -1;
		}

        Mat frame,frame1,frame2,differenceImage,thresholdImage;
        Mat grayImage1,grayImage2;

    while(capture.get(CV_CAP_PROP_POS_FRAMES)<capture.get(CV_CAP_PROP_FRAME_COUNT)-1){

        capture.read(frame);
        capture.read(frame1);
        cv::cvtColor(frame1,grayImage1,COLOR_BGR2GRAY);
        capture.read(frame2);
        cv::cvtColor(frame2,grayImage2,COLOR_BGR2GRAY);

        if(allEnabled)
        {
        objdetectEnabled = true;
        trackingEnabled = true;
        coordinateEnabled = true;
        hogEnabled = true;
        debugMode = true;
        }

        if(allDisabled)
        {
        objdetectEnabled = false;
        trackingEnabled = false;
        coordinateEnabled = false;
        hogEnabled = false;
        debugMode = false;
        }


    //-------------------------------HOG START-----------------------------------

        if(hogEnabled){

            vector<Rect> found, found_filtered;
            hog.detectMultiScale(frame1, found, 0, Size(8,8), Size(32,32), 1.05, 2);

            size_t i, j;
            for (i=0; i<found.size(); i++)
            {
                Rect r = found[i];
                for (j=0; j<found.size(); j++)
                    if (j!=i && (r & found[j])==r)
                        break;
                    if (j==found.size())
                        found_filtered.push_back(r);
                }
                for (i=0; i<found_filtered.size(); i++)
                {
                   Rect r = found_filtered[i];
                   r.x += cvRound(r.width*0.1);
                   r.width = cvRound(r.width*0.8);
                   r.y += cvRound(r.height*0.06);
                   r.height = cvRound(r.height*0.9);
                   rectangle(frame1, r.tl(), r.br(), cv::Scalar(0,0,255),6);
	    //cout<< r.tl();

                   int xh = r.x;
                   int yh = r.y;

                   putText(frame1, "(" + intToString(xh)+","+intToString(yh)+")", r.tl(), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,0,0), 2);
                   if (xh <= 63)
                    {
                    cout << '\a';
                    putText(frame1,"Human in Restricted area",Point(140,30),FONT_HERSHEY_SIMPLEX, 0.5,Scalar(0,255,255),2);
                    }
               }

    	/// c++;
         cout<<"\n\ntotal filter"<<c;
         myfile<<"\n\ntotal filter"<<c;
         cout<<" and number of Rectangled [Filtered]:"<<found_filtered.size();
         myfile<<" and number of Rectangled [Filtered]:"<<found_filtered.size();
           }
	//-----------------------------------HOG ENDS----------------------------------


           cv::absdiff(grayImage1,grayImage2,differenceImage);
           cv::threshold(differenceImage,thresholdImage,SENSITIVITY_VALUE,255,THRESH_BINARY);
            cv::imshow("Init Threshold Image",thresholdImage);

           cv::blur(thresholdImage,thresholdImage,cv::Size(BLUR_SIZE,BLUR_SIZE));
           cv::threshold(thresholdImage,thresholdImage,SENSITIVITY_VALUE,255,THRESH_BINARY);

           if(debugMode==true){
            //show ng the difference image and threshold image
            cv::imshow("Differnce Image",differenceImage);
            cv::imshow("Threshold Image",thresholdImage);

        }else{
            //if not in debug mode, destroy the windows so we don't see them anymore
            cv::destroyWindow("Differnce Image");
            cv::destroyWindow("Threshold Image");
        }


        cv::findContours(thresholdImage,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);

    //------------------------------approxpoly start---------------------------------------------

    if(objdetectEnabled){

        if(trackingEnabled)
        {
            for(unsigned int i=0 ; i<contours.size();i++)
            {
             approxPolyDP(Mat(contours[i]), contours[i], 10, true);

                for(unsigned int j=0;j<contours[i].size();j++)
                {
                    int x = contours[i][j].x;
                    int y = contours[i][j].y;

                    if(coordinateEnabled){
                        putText(frame1,"(" + intToString(x)+","+intToString(y)+")",Point(x,y),0.3,0.3,Scalar(0,255,255),1.3);
                    }
                    circle( frame1, contours[i][j], 3, Scalar(0, 0, 0), 2 );
                  if (y >= 290)
                    {
                    cout << '\a';
                    putText(frame1,"Vehicle in Restricted area",Point(352,322),FONT_HERSHEY_SIMPLEX, 0.5,Scalar(0,255,255),2);
                    }
                }
            }
        }

    //------------------------------approxpoly ends---------------------------------------------


        cv::drawContours(frame1,contours,-1,cv::Scalar(255,255,0),2);
        }

    line( frame1, Point( 63, 3), Point( 63, 232), Scalar( 0, 0, 255 ),  4, 8 );

    writer.write(frame1);
    cv::imshow("MainFrame",frame1);
    c++;
    switch(waitKey(160))
    {
                case 27: //'esc' key has been pressed, exit program.
                return 0;
                case 97: //'a' has been pressed. this will toggle ccordinate
                x = !x;
                if(x == false)
                {
                allDisabled = true;
                allEnabled = false;
                if(allDisabled==true)
                cout<<"All Disabled."<<endl;
                }
                else{
                allDisabled = false;
                allEnabled = true;
                if(allEnabled==true)
                cout<<"All enabled."<<endl;
                }
                break;

                case 99: //'c' has been pressed. this will toggle ccordinate
                coordinateEnabled = !coordinateEnabled;
                if(coordinateEnabled == false) cout<<"Coordinate disabled."<<endl;
                else cout<<"Coordinate enabled."<<endl;
                break;

                case 100: //'d' has been pressed. this will debug mode
                allDisabled = false;
                allEnabled = false;
                debugMode = !debugMode;
                if(debugMode == false) cout<<"Debug mode disabled."<<endl;
                else cout<<"Debug mode enabled."<<endl;
                break;

                case 104:
                allDisabled = false;
                allEnabled = false;
                hogEnabled = !hogEnabled;
                if(hogEnabled == false) cout<<"Human detection disabled."<<endl;
                else cout<<"Human detection enabled."<<endl;
                break;

                case 111: //'o' has been pressed. this will toggle object detection
                allDisabled = false;
                allEnabled = false;
                objdetectEnabled = !objdetectEnabled;
                if(objdetectEnabled == false) cout<<"Object detection disabled."<<endl;
                else cout<<"Object detection enabled."<<endl;
                break;

                case 116: //'t' has been pressed. this will toggle tracking
                allDisabled = false;
                allEnabled = false;
                trackingEnabled = !trackingEnabled;
                if(trackingEnabled == false) cout<<"Tracking disabled."<<endl;
                else cout<<"Tracking enabled."<<endl;
                break;

                case 112: //'p' has been pressed. this will pause/resume the code.
                pause = !pause;
                if(pause == true){ cout<<"Code paused, press 'p' again to resume"<<endl;
                while (pause == true){
                    switch (waitKey()){
                        case 112:
                        pause = false;
                        cout<<"Code resumed."<<endl;
                        break;
                    }
                }
            }
        }
            cout<<c<<endl;
            if(c == 147)
                {
                //return 0;
                }
        }//while2 ends here
            capture.release();
    } //while1 ends here

    return 0;
}
