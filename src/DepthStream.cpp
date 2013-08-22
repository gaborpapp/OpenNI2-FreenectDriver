#include "DepthStream.hpp"

using namespace FreenectDriver;


DepthStream::DepthStream(Freenect::FreenectDevice* pDevice) : VideoStream(pDevice) {
	video_mode = makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30);
    //image_registration_mode = ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR;
    image_registration_mode = ONI_IMAGE_REGISTRATION_OFF;
	setVideoMode(video_mode);
}

// Add video modes here as you implement them
// Note: if image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR,
// setVideoFormat() will try FREENECT_DEPTH_REGISTERED first then fall back on what is set here.
DepthStream::FreenectDepthModeMap DepthStream::getSupportedVideoModes() {
	FreenectDepthModeMap modes;
	//											pixelFormat, resolutionX, resolutionY, fps		freenect_video_format, freenect_resolution						
	modes[makeOniVideoMode(ONI_PIXEL_FORMAT_DEPTH_1_MM, 640, 480, 30)] = std::pair<freenect_depth_format, freenect_resolution>(FREENECT_DEPTH_MM, FREENECT_RESOLUTION_MEDIUM);
	
	
	return modes;
}

OniStatus DepthStream::setVideoMode(OniVideoMode requested_mode) {
	FreenectDepthModeMap supported_video_modes = getSupportedVideoModes();
	FreenectDepthModeMap::const_iterator matched_mode_iter = supported_video_modes.find(requested_mode);
	if (matched_mode_iter == supported_video_modes.end())
		return ONI_STATUS_NOT_SUPPORTED;      
	
	freenect_depth_format format = matched_mode_iter->second.first;
	freenect_resolution resolution = matched_mode_iter->second.second;
	if (image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) // try forcing registration mode
		format = FREENECT_DEPTH_REGISTERED;
	
	try { device->setDepthFormat(format, resolution); }
	catch (std::runtime_error e) {
		printf("format-resolution combination not supported by libfreenect: %d-%d\n", format, resolution);
		if (image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) {
			printf("could not use image registration format; disabling registration and falling back to format defined in getSupportedVideoModes()\n");
			image_registration_mode = ONI_IMAGE_REGISTRATION_OFF;
			return setVideoMode(requested_mode);
		}
		return ONI_STATUS_NOT_SUPPORTED;
	}
	video_mode = requested_mode;
	return ONI_STATUS_OK;
}

// Discard the depth value equal or greater than the max value.
inline unsigned short filterReliableDepthValue(unsigned short value)
{
    return value < DepthStream::MAX_VALUE ? value : 0;
}

void DepthStream::populateFrame(void* data, OniFrame* frame) const
{
    if (!_cropping.enabled)
    {   // no croping
        frame->cropOriginX = frame->cropOriginY = 0;
        frame->croppingEnabled = FALSE;
    }
    else
    {   // croping
        frame->height = _cropping.height;
        frame->width  = _cropping.width;
        frame->cropOriginX = _cropping.originX;
        frame->cropOriginY = _cropping.originY;
        frame->croppingEnabled = TRUE;
    }

    frame->sensorType = sensor_type;
	frame->stride = video_mode.resolutionX*sizeof(uint16_t);

    int numPoints = video_mode.resolutionY * video_mode.resolutionX;

    // populate the pixel data
    copyDepthPixelsStraight((unsigned short*)data, numPoints, frame);
}

void DepthStream::copyDepthPixelsStraight(unsigned short* source, int numPoints, OniFrame* pFrame) const
{
    unsigned short* target = (unsigned short*) pFrame->data;

    const unsigned int width = pFrame->width;
    const unsigned int height = pFrame->height;
    const unsigned int skipWidth = video_mode.resolutionX - width;

    // Offset the starting position
    source += pFrame->cropOriginX + pFrame->cropOriginY * video_mode.resolutionX;

    if(mirroring)
    {
        target = target + width;

        for (unsigned int y = 0; y < height; y++)
        {
            for (unsigned int x = 0; x < width; x++)
                *(target--) = filterReliableDepthValue(*(source++));

            source += skipWidth;
            target += 2 * width;
        }
    }
    else
    {
        for (unsigned int y = 0; y < height; y++)
        {
            for (unsigned int x = 0; x < width; x++)
                *(target++) = filterReliableDepthValue(*(source++));

            source += skipWidth;
        }
    }
}


/* depth video modes reference

FREENECT_DEPTH_11BIT        = 0, //< 11 bit depth information in one uint16_t/pixel
FREENECT_DEPTH_10BIT        = 1, //< 10 bit depth information in one uint16_t/pixel
FREENECT_DEPTH_11BIT_PACKED = 2, //< 11 bit packed depth information
FREENECT_DEPTH_10BIT_PACKED = 3, //< 10 bit packed depth information
FREENECT_DEPTH_REGISTERED   = 4, //< processed depth data in mm, aligned to 640x480 RGB
FREENECT_DEPTH_MM           = 5, //< depth to each pixel in mm, but left unaligned to RGB image
FREENECT_DEPTH_DUMMY        = 2147483647, //< Dummy value to force enum to be 32 bits wide

ONI_PIXEL_FORMAT_DEPTH_1_MM = 100,
ONI_PIXEL_FORMAT_DEPTH_100_UM = 101,
ONI_PIXEL_FORMAT_SHIFT_9_2 = 102,
ONI_PIXEL_FORMAT_SHIFT_9_3 = 103,
*/
