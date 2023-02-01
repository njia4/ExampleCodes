#include <iostream>
#include <unistd.h>
#include <libuvc/libuvc.h>

using namespace std;

int verbose = 1;

uvc_context_t *uvc_context_manager_;
uvc_device_t *uvc_device_;
uvc_device_handle_t *uvc_device_handle_;
uvc_stream_ctrl_t uvc_ctrl;
uvc_error_t res;

void cb(uvc_frame_t *frame, void *ptr)
{
	cout << "Received frame " << frame->sequence << endl;
}

int main(int argc, char const *argv[])
{
	// Initialize the context manager
	res = uvc_init(&uvc_context_manager_, NULL);
	if (res<0)
		throw runtime_error("Failed to initialize the UVC context manager!");
	if (verbose)
		cout << "UVC context manager initialized. " << endl;

	// Find device
	res = uvc_find_device(uvc_context_manager_, &uvc_device_, 0, 0, NULL);
	if (res<0)
		uvc_strerror(res);
	if (verbose)
		cout << "UVC device found." << endl;

	// Open device
	res = uvc_open(uvc_device_, &uvc_device_handle_);
	if (res<0)
		throw runtime_error(uvc_strerror(res));
	if (verbose)
		cout << "UVC device opened. " << endl;

	// Pring devices infos
	if (verbose)
		uvc_print_diag(uvc_device_handle_, stderr);

	/* ================= */
	/* Setup Stream Info */
	/* ================= */
	// Get frame and format info from the device
	const uvc_format_desc_t *format_desc = uvc_get_format_descs(uvc_device_handle_);
	const uvc_frame_desc_t *frame_desc = format_desc->frame_descs;
	enum uvc_frame_format frame_format;
	int width, height ,fps;

	frame_format = UVC_COLOR_FORMAT_YUYV; // I just know this is true for HT-301
	if (frame_desc) {
		width = frame_desc->wWidth;
		height = frame_desc->wHeight;
		fps = 10000000 / frame_desc->dwDefaultFrameInterval;
	}

	// Try to negotiate first stream profile
	res = uvc_get_stream_ctrl_format_size(
		uvc_device_handle_, &uvc_ctrl, /* result stored in ctrl */
		frame_format,
		width, height, fps /* width, height, fps */
	);
 	if (res<0)
 		throw runtime_error(uvc_strerror(res));
 	if (verbose)
 		uvc_print_stream_ctrl(&uvc_ctrl, stderr);

 	/* =============== */
 	/* Streaming Start */
 	/* =============== */
 	res = uvc_start_streaming(uvc_device_handle_, &uvc_ctrl, cb, (void *) 12345, 0);
 	if (res<0)
 		throw runtime_error(uvc_strerror(res));
 	if (verbose)
 		cout << "UVC stream started. " << endl;

 	sleep(5);

 	uvc_stop_streaming(uvc_device_handle_);
 	if (verbose)
 		cout << "UVC stream stopped. " << endl;

	// Close device
	uvc_close(uvc_device_handle_);
	uvc_unref_device(uvc_device_);
	if (verbose)
		cout << "UVC device closed. " << endl;

	// Close context manager
	uvc_exit(uvc_context_manager_);
	
	return 0;
}