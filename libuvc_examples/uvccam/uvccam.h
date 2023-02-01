/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2022, Jia
 *
 * uvc_cam.h - base class for UVC cameras.
 */

#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include <functional>
#include <libuvc/libuvc.h>

using namespace std;

class uvccam
{
public:
	uvccam();
	~uvccam();

	enum mode
	{
		IDLE,
		NORMAL,
		OVERLAY,
		EDGE,
	};

	void setupStream();
	void startCapture();
	void stopCapture();
	int setMode(mode m);
	void setCallback(std::function<void()> callback) { showBuffer = callback; };
	void configBuffer(void* vaddr) { display_bo_addr_ = vaddr; };
	void increaseCutoff(void) { cutoff += cutoff_delta; };
	void decreaseCutoff(void) { cutoff -= cutoff_delta; };

private:
	// void captureThread();
	void convertFormat(uvc_frame_t *frame);
	static void cb(uvc_frame_t *frame, void *ptr);

	int verbose = 1;
	// Running mode
	mode running_mode = IDLE;
	// Capture
	int camera_id;
	// Camera information
	// TODO: GET THOSE NUMBERS FROM THE CAMERA
	int width = 384;
	int height = 292;

	// UVC context manager and device objects
	uvc_context_t *uvc_context_manager_;
	uvc_device_t *uvc_device_;
	uvc_device_handle_t *uvc_device_handle_;
	uvc_stream_ctrl_t uvc_ctrl;
	uvc_error_t res;
	void* display_bo_addr_;
	std::function<void()> showBuffer;
	int cutoff = 50; // Itensity cutoff for overlay display
	int cutoff_delta = 10;
	
	// // Frames
	// void* frame_data[384*292/2]; 
	// Mat frame_YUYV = Mat(292, 384, CV_8UC2 , frame_data);
	// Mat frame_Mono16 = Mat(292, 384, CV_16UC1 , frame_data); // Alias of the same frame buffer as frame_YUYV
	// Mat frame_rgb = Mat(292, 384, CV_8UC3); // For direct showing the thermal image
	// Mat frame_bgra;
	// // Auxillary Mat for edge detection
	// Mat one_mat = Mat::ones(height, width, CV_8UC1);
	// Mat zero_mat = Mat::zeros(height, width, CV_8UC1);
	// Mat channel_a;
	// Mat channel_r;

	// Capture thread
	// std::thread capture_thread_;
	// bool capture_abort_;
	// Display buffer
	// std::shared_ptr<buffer_object> buffer_;
	// std::function<void()> showBuffer;
	// Parameters
	// int cutoff = 2500;
	// int inc = 2;
	// float scale = 1.0;
};
