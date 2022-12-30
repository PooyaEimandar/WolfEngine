/*
    Project: Wolf Engine. Copyright © 2014-2022 Pooya Eimandar
    https://github.com/WolfEngine/WolfEngine
*/

#ifdef WOLF_MEDIA_FFMPEG

#pragma once

#include <wolf.hpp>

#include <DISABLE_ANALYSIS_BEGIN>
extern "C" {
#include <libavcodec/packet.h>
}
#include <DISABLE_ANALYSIS_END>

namespace wolf::media::ffmpeg {

struct w_decoder;
struct w_encoder;
struct w_ffmpeg;

class w_av_packet {
  friend w_decoder;
  friend w_encoder;
  friend w_ffmpeg;

public:
  /**
   * construct an av_packet
   */
  W_API w_av_packet() noexcept;

  /**
   * construct an av_packet
   */
  W_API explicit w_av_packet(AVPacket* p_av_packet) noexcept;

  // move constructor.
  W_API w_av_packet(w_av_packet &&p_other) noexcept = default;
  // move assignment operator.
  W_API w_av_packet &operator=(w_av_packet &&p_other) noexcept = default;

  // destructor
  W_API virtual ~w_av_packet() noexcept;

  /**
   * initialize the av_packet
   * @returns zero on success
   */
  W_API boost::leaf::result<int> init() noexcept;

  /**
   * initialize the av_packet from data
   * @returns zero on success
   */
  W_API boost::leaf::result<int> init(_In_ uint8_t *p_data,
                                      _In_ size_t p_data_len) noexcept;

  /**
   * unref av_packet
   */
  W_API void unref() noexcept;

  // get packet data
  W_API uint8_t* get_data() const noexcept;

  // get packet size
  W_API int get_size() const noexcept;

  // get stream index
  W_API int get_stream_index() const noexcept;

private:
  // copy constructor.
  w_av_packet(const w_av_packet &) = delete;
  // copy assignment operator.
  w_av_packet &operator=(const w_av_packet &) = delete;

  void _release() noexcept;

  AVPacket* _packet = nullptr;
};
} // namespace wolf::media::ffmpeg

#endif