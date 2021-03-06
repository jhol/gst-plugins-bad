/* GStreamer
 * Copyright (C) 2020 Seungha Yang <seungha@centricular.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_MEDIA_CAPTURE_WRAPPER_H__
#define __GST_MEDIA_CAPTURE_WRAPPER_H__

#include <gst/gst.h>
#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.media.capture.h>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <condition_variable>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::Media::Capture;
using namespace ABI::Windows::Media::Capture::Frames;
using namespace ABI::Windows::Graphics::Imaging;

/* Store Format info and its caps representation */
class GstWinRTMediaDescription
{
public:
  GstWinRTMediaDescription();
  GstWinRTMediaDescription(const GstWinRTMediaDescription& other);
  ~GstWinRTMediaDescription();
  void Release();
  bool IsValid() const;
  HRESULT Fill(HString &source_id,
               const ComPtr<IMediaCaptureVideoProfileMediaDescription>& desc);

  GstWinRTMediaDescription& operator=(const GstWinRTMediaDescription& rhs)
  {
    if (this == &rhs)
      return *this;

    Release();
    if (rhs.source_id_.IsValid())
      rhs.source_id_.CopyTo(source_id_.GetAddressOf());
    if (rhs.subtype_.IsValid())
      rhs.subtype_.CopyTo(subtype_.GetAddressOf());
    gst_caps_replace (&caps_, rhs.caps_);

    return *this;
  }

public:
  HString source_id_;
  /* TODO: need to cover audio too */
  HString subtype_;
  /* Source ID which is mapped to MediaFormatSource */
  GstCaps *caps_;
};

/* holds GstWinRTMediaFrameSourceInfo, corresponding to per device info */
class GstWinRTMediaFrameSourceGroup
{
public:
  GstWinRTMediaFrameSourceGroup();
  GstWinRTMediaFrameSourceGroup(const GstWinRTMediaFrameSourceGroup& other);
  ~GstWinRTMediaFrameSourceGroup();
  void Release();
  bool Contain(const GstWinRTMediaDescription &desc);
  HRESULT Fill(const ComPtr<IMediaFrameSourceGroup> &source_group);

  GstWinRTMediaFrameSourceGroup& operator=(const GstWinRTMediaFrameSourceGroup& rhs)
  {
    if (this == &rhs)
      return *this;

    Release();
    id_ = rhs.id_;
    display_name_ = rhs.display_name_;
    source_group_ = rhs.source_group_;
    source_list_ = rhs.source_list_;

    return *this;
  }

public:
  std::string id_;
  std::string display_name_;
  ComPtr<IMediaFrameSourceGroup> source_group_;
  std::vector<GstWinRTMediaDescription> source_list_;
};

typedef struct
{
  HRESULT (*frame_arrived) (ISoftwareBitmap * bitmap, void * user_data);
  HRESULT (*failed)        (const std::string &error,
                            UINT32 error_code,
                            void * user_data);
} MediaCaptureWrapperCallbacks;
class MediaCaptureWrapper
{
public:
  MediaCaptureWrapper();
  ~MediaCaptureWrapper();

  void RegisterCb(const MediaCaptureWrapperCallbacks &cb,
                  void * user_data);

  /* Enumerating available source devices */
  /* Fill enumerated device infos into list */
  HRESULT EnumrateFrameSourceGroup(std::vector<GstWinRTMediaFrameSourceGroup> &group_list);
  /* Select target device which should be one of enumerated be fore */
  HRESULT SetSourceGroup(const GstWinRTMediaFrameSourceGroup &group);
  /* Select target format (resolution, video format) to use */
  HRESULT SetMediaDescription(const GstWinRTMediaDescription &desc);

  /* Start and Stop capturing operation */
  HRESULT StartCapture();
  HRESULT StopCapture();
  HRESULT GetAvailableDescriptions(std::vector<GstWinRTMediaDescription> &desc_list);

private:
  ComPtr<IMediaCapture> media_capture_;
  ComPtr<IMediaFrameReader> frame_reader_;
  ComPtr<ICoreDispatcher> dispatcher_;
  bool init_done_;
  std::mutex lock_;
  std::condition_variable cond_;

  EventRegistrationToken token_frame_arrived_;
  EventRegistrationToken token_capture_failed_;

  std::unique_ptr<GstWinRTMediaFrameSourceGroup> source_group_;
  std::unique_ptr<GstWinRTMediaDescription> media_desc_;
  MediaCaptureWrapperCallbacks user_cb_;
  void *user_data_;

private:
  HRESULT openMediaCapture();
  HRESULT mediaCaptureInitPre();
  HRESULT mediaCaptureInitPost(ComPtr<IAsyncAction> init_async,
                               ComPtr<IMediaCapture> media_capture);
  HRESULT startCapture();
  HRESULT stopCapture();
  HRESULT onFrameArrived(IMediaFrameReader *reader,
                         IMediaFrameArrivedEventArgs *args);
  HRESULT onCaptureFailed(IMediaCapture *capture,
                          IMediaCaptureFailedEventArgs *args);
  void findCoreDispatcher();
  static HRESULT enumrateFrameSourceGroup(std::vector<GstWinRTMediaFrameSourceGroup> &list);

  template <typename CB>
  HRESULT runOnUIThread(DWORD timeout, CB && cb)
  {
    ComPtr<IAsyncAction> asyncAction;
    HRESULT hr;
    HRESULT hr_callback;
    boolean can_now;
    DWORD wait_ret;

    if (!dispatcher_)
      return cb();

    hr = dispatcher_->get_HasThreadAccess (&can_now);

    if (FAILED (hr))
      return hr;

    if (can_now)
      return cb ();

    Event event (CreateEventEx (NULL, NULL, CREATE_EVENT_MANUAL_RESET,
        EVENT_ALL_ACCESS));

    if (!event.IsValid())
      return E_FAIL;

    auto handler =
        Callback<Implements<RuntimeClassFlags<ClassicCom>,
            IDispatchedHandler, FtmBase>>([&hr_callback, &cb, &event] {
          hr_callback = cb ();
          SetEvent (event.Get());
          return S_OK;
        });

    hr = dispatcher_->RunAsync(CoreDispatcherPriority_Normal,
        handler.Get(), &asyncAction);

    if (FAILED (hr))
      return hr;

    wait_ret = WaitForSingleObject(event.Get(), timeout);
    if (wait_ret != WAIT_OBJECT_0)
      return E_FAIL;

    return hr_callback;
  }
};

#endif /* __GST_MEDIA_CAPTURE_WRAPPER_H__ */