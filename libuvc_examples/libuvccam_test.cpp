#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <preview.h>
#include <uvccam/uvccam.h>

using namespace std;

drmPreview display;
uvccam camera;

int main(int argc, char const *argv[])
{
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
	int plane_id = display.addPlane(buf_, DRM_DUMB_BUFFER);

	camera.configBuffer(buf_->vaddr);
	camera.setMode(uvccam::NORMAL);
	camera.setCallback([plane_id] () { display.showPlane(plane_id); });
	camera.setupStream();
	camera.startCapture();

	int c;
	while (true)
	{
		c = getchar();

		if (c=='n')
			camera.setMode(uvccam::NORMAL);
		if (c=='o')
			camera.setMode(uvccam::OVERLAY);
		if (c=='p')
			camera.increaseCutoff();
		if (c=='m')
			camera.decreaseCutoff();
		if (c=='e')
			break;
	}
	
	camera.stopCapture();

	return 0;
}