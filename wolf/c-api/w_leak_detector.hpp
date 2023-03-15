/*
    Project: Wolf Engine. Copyright © 2014-2023 Pooya Eimandar
    https://github.com/WolfEngine/WolfEngine
*/

#pragma once

#if defined(WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#endif

#include <crtdbg.h>

class w_leak_detector {
 public:
  // default constructor
  w_leak_detector() noexcept;
  // destructor
  virtual ~w_leak_detector() noexcept;

 private:
  // move constructor
  w_leak_detector(w_leak_detector &&p_src) noexcept = delete;
  // move assignment operator.
  w_leak_detector &operator=(w_leak_detector &&p_src) noexcept = delete;
  // copy constructor.
  w_leak_detector(const w_leak_detector &) = delete;
  // copy assignment operator.
  w_leak_detector &operator=(const w_leak_detector &) = delete;

#if defined(WIN32) && defined(_DEBUG)
  _CrtMemState _mem_state;
#endif
};