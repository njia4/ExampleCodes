/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2022, Jia
 *
 * uvc_cam.cpp - base class for UVC cameras.
 */

#include <uvccam.h>
#include <iostream>

static inline unsigned char sat(int i) {
  return (unsigned char)( i >= 255 ? 255 : (i < 0 ? 0 : i));
}

void CONVERT_NORMAL(uint8_t* pin, uint8_t* pout, int cutoff) 
{
    int v = (pin)[0] + 0;
    (pout)[0] = sat(v); // B
    (pout)[1] = sat(v); // G
    (pout)[2] = sat(v); // R
    (pout)[3] = sat(255); // A
}

void CONVERT_OVERLAY(uint8_t* pin, uint8_t* pout, int cutoff)
{
    int v = (pin)[0] - cutoff;
    (pout)[0] = sat(0); // B
    (pout)[1] = sat(13*(v/20)); // G
    (pout)[2] = sat(v); // R
    (pout)[3] = sat(2*v); // A
    }

uvccam::uvccam()
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
}

uvccam::~uvccam()
{
    // Close device
    uvc_close(uvc_device_handle_);
    uvc_unref_device(uvc_device_);
    if (verbose)
        cout << "UVC device closed. " << endl;

    // Close context manager
    uvc_exit(uvc_context_manager_);
}

void uvccam::setupStream()
{
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
}

void uvccam::startCapture()
{
    res = uvc_start_streaming(uvc_device_handle_, &uvc_ctrl, &uvccam::cb, this, 0);
    if (res<0)
    {
        cout << "Failed to start the stream." << endl;
        throw runtime_error(uvc_strerror(res));
    }
    if (verbose)
        cout << "UVC stream started. " << endl;
}

void uvccam::stopCapture()
{
    uvc_stop_streaming(uvc_device_handle_);
}

int uvccam::setMode(mode m)
{
    if (verbose)
        std::cout << "Switching mode: " << m << std::endl;

    running_mode = m;

    // if (running_mode==IDLE)
    //     stopCapture();

    // TODO: START CAPTURE IF NO STREAM

    return 1;
}

void uvccam::cb(uvc_frame_t *frame, void *ptr)
{
    uvccam *cam = static_cast<uvccam*>(ptr);
    cam->convertFormat(frame);
    cam->showBuffer();
}

void uvccam::convertFormat(uvc_frame_t *frame)
{
    uint8_t *ptr_in  = (uint8_t*) frame->data;
    uint8_t *ptr_out = (uint8_t*) display_bo_addr_;
    uint8_t *ptr_in_end = ptr_in + frame->data_bytes;

    while (ptr_in < ptr_in_end) 
    {
        switch (running_mode)
        {
            case NORMAL:
                CONVERT_NORMAL(ptr_in, ptr_out, cutoff);
                break;
            case OVERLAY:
                CONVERT_OVERLAY(ptr_in, ptr_out, cutoff);
                break;
        }

        ptr_in  += 2;
        ptr_out += 4 ;
    }
}
