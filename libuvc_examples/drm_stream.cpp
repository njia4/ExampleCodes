#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <preview.h>
#include <libuvc/libuvc.h>

using namespace std;

int verbose = 1;

/* preview variable */
drmPreview display;
void* bo_data_addr;
int plane_id;
/* End preview variable */

/* libuvc variable */
uvc_context_t *uvc_context_manager_;
uvc_device_t *uvc_device_;
uvc_device_handle_t *uvc_device_handle_;
uvc_stream_ctrl_t uvc_ctrl;
uvc_error_t res;
int cutoff = 50;
/* End libuvc variable */

static inline unsigned char sat(int i) {
  return (unsigned char)( i >= 255 ? 255 : (i < 0 ? 0 : i));
}

void convert_format(uvc_frame_t *frame, void *bo_data)
{
	uint8_t *ptr_in  = (uint8_t*) frame->data;
	uint8_t *ptr_out = (uint8_t*) bo_data;
	uint8_t *ptr_in_end = ptr_in + frame->data_bytes;

	while (ptr_in < ptr_in_end) 
	{
		// int v = (256*(ptr_in)[0]+(ptr_in)[1]) - 5000;
		// int v = ((ptr_in)[0]-128) + 256 * ((ptr_in)[1]-128);
		int v = (ptr_in)[0] - cutoff;

		// uint8_t v0 = (ptr_in)[0]+0;
		// uint8_t v1 = (ptr_in)[1]+0;
		// printf ("%02x, %02x \n", v1, v0);
		// cout << "(0, " << 2*13*(v/20) << ", " << 2*v << ", " << v << ")" << endl;
		(ptr_out)[0] = sat(0); // B
		(ptr_out)[1] = sat(13*(v/20)); // G
		(ptr_out)[2] = sat(v); // R
		(ptr_out)[3] = sat(2*v); // A

		ptr_in  += 2;
		ptr_out += 4 ;
	}
}

void cb(uvc_frame_t *frame, void *ptr)
{
	convert_format(frame, bo_data_addr);
	display.showPlane(plane_id);
}

int main(int argc, char const *argv[])
{
	cutoff = atoi(argv[1]);
	cout << "Cutoff: " << atoi(argv[1]) << endl;
	/*==================*/
	/*== preview part ==*/
	/*==================*/
	std::shared_ptr<drm_buffer> buf_ = display.makeBuffer();
	buf_->width = 384;
	buf_->height = 292;
	buf_->bpp = 32;
	buf_->pixel_format = DRM_FORMAT_ABGR8888;
	buf_->roi_x = 0;
	buf_->roi_y = 0;
	buf_->roi_w = 1;
	buf_->roi_h = 1;
	buf_->display_x = 0;
	buf_->display_y = 0;
	buf_->display_w = 1;
	buf_->display_h = 1;
	plane_id = display.addPlane(buf_, DRM_DUMB_BUFFER);
	bo_data_addr = buf_->vaddr;

	/*=================*/
	/*== libuvc part ==*/
	/*=================*/
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

 	getchar();

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