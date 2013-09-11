#include "InfraRedStream.hpp"

using namespace FreenectDriver;


InfraRedStream::InfraRedStream(Freenect::FreenectDevice* pDevice) : VideoStream(pDevice) {
  video_mode = makeOniVideoMode(ONI_PIXEL_FORMAT_GRAY8, 640, 480, 30);
  setVideoMode(video_mode);
}

// Add video modes here as you implement them
InfraRedStream::FreenectVideoModeMap InfraRedStream::getSupportedVideoModes() {
  FreenectVideoModeMap modes;
  //                    pixelFormat, resolutionX, resolutionY, fps    freenect_video_format, freenect_resolution
  modes[makeOniVideoMode(ONI_PIXEL_FORMAT_GRAY8, 640, 480, 30)] = std::pair<freenect_video_format, freenect_resolution>(FREENECT_VIDEO_IR_8BIT, FREENECT_RESOLUTION_MEDIUM);


  return modes;
}

OniStatus InfraRedStream::setVideoMode(OniVideoMode requested_mode) {
    FreenectVideoModeMap supported_video_modes = getSupportedVideoModes();
    FreenectVideoModeMap::const_iterator matched_mode_iter = supported_video_modes.find(requested_mode);
    if (matched_mode_iter == supported_video_modes.end())
        return ONI_STATUS_NOT_SUPPORTED;

    freenect_video_format format = matched_mode_iter->second.first;
    freenect_resolution resolution = matched_mode_iter->second.second;

    try { device->setVideoFormat(format, resolution); }
    catch (std::runtime_error e) {
        printf("format-resolution combination not supported by libfreenect: %d-%d\n", format, resolution);
        return ONI_STATUS_NOT_SUPPORTED;
    }
    video_mode = requested_mode;
    return ONI_STATUS_OK;
}

void InfraRedStream::populateFrame(void* data, OniFrame* frame) const {
  frame->sensorType = sensor_type;
  frame->stride = video_mode.resolutionX;
  frame->cropOriginX = frame->cropOriginY = 0;
  frame->croppingEnabled = FALSE;

  // copy stream buffer from freenect
  switch (video_mode.pixelFormat) {
    default:
      printf("pixelFormat %d not supported by populateFrame\n", video_mode.pixelFormat);
      return;

    case ONI_PIXEL_FORMAT_GRAY8:
      unsigned char* source = static_cast<unsigned char*>(data);
      unsigned char* target = static_cast<unsigned char*>(frame->data);
      if (mirroring)
      {
          // go to the end of this line
          target = target + (video_mode.resolutionX -1);

          for (unsigned int y = 0; y < video_mode.resolutionY; y++)
          {
              for (unsigned int x = 0; x < video_mode.resolutionX; x++)
                  *(target--)   = *(source++);

              target += 2 * video_mode.resolutionX;
          }

          /*
          for (unsigned int i = 0; i < frame->dataSize; i++)
          {
              // find corresponding mirrored pixel
              unsigned int row = i / video_mode.resolutionX;
              unsigned int col = video_mode.resolutionX - (i % video_mode.resolutionX);
              unsigned int target = (row * video_mode.resolutionX) + col;
              // copy it to this pixel
              frame_data[i] = data_ptr[target];
          }
          */
      }
      else
          memcpy(target,source,frame->dataSize);

      return;
  }
}
