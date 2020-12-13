#include <opencv2/opencv.hpp>
#include <vector>
#include <stdio.h>
using namespace std;
using namespace cv;

inline Point GetCenter(const Rect& rc)
{
	return{ rc.x + rc.width / 2, rc.y + rc.height / 2 };
}
void RemoveColor(Mat& src, Mat& dst);
vector<Rect> getLabelRegion(Mat& labeled, int labelCount);

int main()
{
	VideoCapture video("번호판.mp4");
	int frame = video.get(CAP_PROP_FRAME_COUNT);
	int fps = video.get(CAP_PROP_FPS);
	int spf = 1000 / fps;
	for (int i = 0; i < frame; ++i)
	{
		Mat color;
		video >> color;
		resize(color, color, Size(640, 360));

		Mat removed;
		RemoveColor(color, removed);
		imshow("removed", removed);

		Mat gray;
		cvtColor(removed, gray, COLOR_BGR2GRAY);
		imshow("gray", gray);

		Mat binary;
		int thres = 100; //50
		threshold(gray, binary, thres, 100, THRESH_BINARY); //원래는 255값을 가지고 있었
		imshow("binary", binary);

		Mat labeled;
		int labelCount = connectedComponents(binary, labeled, 8, CV_16U);
		auto rects = getLabelRegion(labeled, labelCount);

		
		float total_area = color.rows * color.cols;
		Point camera_center = { color.cols / 2, color.rows / 2 };

		vector<Rect> remainRects; // 남은 사각형들 넣기

		for (auto& rc : rects) //rc 는 rects의 요소들 순서대로 접근
		{
			///////////
			//for (std::vector<Rect>::iterator iter = remainRects.begin(); iter != remainRects.end(); )
			//{
			//	double rate = (*iter).width / (*iter).height;
			//	if (rate <= 0.8)
			//	{
			//		std::cout << "삭제" << endl;
			//		iter = remainRects.erase(iter);
			//	}
			//	else
			//		++iter;
			//}
			////////
			
			if (rc.area() <= 0) continue;
			remainRects.push_back(rc);
			if (total_area / 2 <= rc.area()) continue;
			if (rc.width > 200 || rc.width <20) continue;

			double rate = (rc.width / rc.height);
			if (rate <= 3.5 || rate >= 15.0) continue;

			rectangle(color, rc, Scalar(0, 255, 255),3); //노랑
			if (total_area * 0.0001 > rc.area()) continue;
			float ratio = rc.width / rc.height;
			if ((4 <= ratio && ratio <= 6) == false) continue;
			Point center = { rc.x + rc.width / 2, rc.y + rc.height / 2 };
			if (center.y < (color.rows / 2.5)) continue;
			if (abs(camera_center.x - center.x) > color.cols / 8) continue;

			
		}
		imshow("color", color);
		waitKey(spf);
	}
	waitKey(0);
	return 0;
}
void RemoveColor(Mat& src, Mat& dst)
{//스레쉬올드
	cvtColor(src, dst, COLOR_BGR2HSV);
	int sThres = 100; // 50
	int vThres = 170; //70
	for (int y = 0; y < dst.rows; ++y)
	{
		for (int x = 0; x < dst.cols; ++x)
		{
			auto hsv = dst.at<Vec3b>({ x, y });
			int h = hsv[0];
			int s = hsv[1];
			int v = hsv[2];
			
			Vec3b newColor = src.at<Vec3b>({ x, y });;
			if (s < sThres && v < vThres)
			{
				newColor = { 0, 0, 0 };
			}
			else if (s <sThres && v > vThres)
			{
			}
			else
			{
				newColor = { 0, 0, 0 };
			}
			dst.at<Vec3b>({ x, y }) = newColor;
		}
	}
}
vector<Rect> getLabelRegion(Mat& labeled, int labelCount)
{
	int width = labeled.cols;
	int height = labeled.rows;
	vector<Rect> rects(labelCount, {
		width, height,
		-width, -height
	});
	int left, top, right, bottom;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			auto label = labeled.at<ushort>({ x, y }); // 전달 받은 벡터의 요소중 하나
			auto& rect = rects[label]; // 전달 받은 벡터의 요소의 주소값
			// 사각형 네 면의 길이 구하기
			left = rect.x;
			top = rect.y;
			right = rect.width + rect.x;
			bottom = rect.height + rect.y;
			if (left > x)
				left = x;

			if (right < x)
				right = x;

			if (top > y)
				top = y;

			if (bottom < y)
				bottom = y;

			rect.x = left;
			rect.y = top;
			rect.width = right - left;
			rect.height = bottom - top;
		}
	}
	return rects;
}