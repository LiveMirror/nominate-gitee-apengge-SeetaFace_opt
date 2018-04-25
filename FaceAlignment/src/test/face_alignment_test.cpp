/*
 *
 * This file is part of the open-source SeetaFace engine, which includes three modules:
 * SeetaFace Detection, SeetaFace Alignment, and SeetaFace Identification.
 *
 * This file is an example of how to use SeetaFace engine for face alignment, the
 * face alignment method described in the following paper:
 *
 *
 *   Coarse-to-Fine Auto-Encoder Networks (CFAN) for Real-Time Face Alignment, 
 *   Jie Zhang, Shiguang Shan, Meina Kan, Xilin Chen. In Proceeding of the
 *   European Conference on Computer Vision (ECCV), 2014
 *
 *
 * Copyright (C) 2016, Visual Information Processing and Learning (VIPL) group,
 * Institute of Computing Technology, Chinese Academy of Sciences, Beijing, China.
 *
 * The codes are mainly developed by Jie Zhang (a Ph.D supervised by Prof. Shiguang Shan)
 *
 * As an open-source face recognition engine: you can redistribute SeetaFace source codes
 * and/or modify it under the terms of the BSD 2-Clause License.
 *
 * You should have received a copy of the BSD 2-Clause License along with the software.
 * If not, see < https://opensource.org/licenses/BSD-2-Clause>.
 *
 * Contact Info: you can send an email to SeetaFace@vipl.ict.ac.cn for any problems.
 *
 * Note: the above information must be kept whenever or wherever the codes are used.
 *
 */

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#include "opencv.hpp"
#include "cv.h"
#include "highgui.h"

#include "face_detection.h"
#include "face_alignment.h"

#ifdef _WIN32
std::string DATA_DIR = "../../data/";
std::string MODEL_DIR = "../../model/";
#else
std::string DATA_DIR = "./data/";
std::string MODEL_DIR = "./model/";
#endif

int main(int argc, char** argv)
{
  // Initialize face detection model
  seeta::FaceDetection detector("../../../FaceDetection/model/seeta_fd_frontal_v1.0.bin");
  detector.SetMinFaceSize(70);
  detector.SetScoreThresh(2.f);
  detector.SetImagePyramidScaleFactor(0.8f);
  detector.SetWindowStep(4, 4);

  // Initialize face alignment model 
  seeta::FaceAlignment point_detector((MODEL_DIR + "seeta_fa_v1.1.bin").c_str());

  cv::Mat img = cv::imread((DATA_DIR + "0_1_2.jpg").c_str(), cv::IMREAD_UNCHANGED);
  cv::Mat img_gray;

  if (img.channels() != 1)
	  cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
  else
	  img_gray = img;

  seeta::ImageData img_data;
  img_data.data = img_gray.data;
  img_data.width = img_gray.cols;
  img_data.height = img_gray.rows;
  img_data.num_channels = 1;



  // Detect faces
  std::vector<seeta::FaceInfo> faces = detector.Detect(img_data);
  int32_t face_num = static_cast<int32_t>(faces.size());

  if (face_num == 0)
  {
    return 0;
  }

  long t0 = cv::getTickCount();
  for (int k = 0; k < 1000; k++)
  {
	  for each (seeta::FaceInfo face in faces)
	  {
		  // Detect 5 facial landmarks
		  seeta::FacialLandmark points[5];
		  point_detector.PointDetectLandmarks(img_data, face, points);

		  // Visualize the results
		  rectangle(img, cvPoint(face.bbox.x, face.bbox.y), cvPoint(face.bbox.x + face.bbox.width - 1, face.bbox.y + face.bbox.height - 1), CV_RGB(255, 0, 0));
		  for (int i = 0; i<5; i++)
		  {
			  circle(img, cvPoint(points[i].x, points[i].y), 2, CV_RGB(0, 255, 0), CV_FILLED);
		  }
	  }
  }

  long t1 = cv::getTickCount();
  double secs = (t1 - t0) / cv::getTickFrequency();
  std::cout << "alignment takes " << secs / (faces.size() * 1000) << " seconds " << std::endl;
  cv::namedWindow("Test", cv::WINDOW_AUTOSIZE);
  cv::imshow("Test", img);
  cv::waitKey(0);
  cv::destroyAllWindows();

  return 0;
}
