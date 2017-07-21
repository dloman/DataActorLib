#pragma once

#include <DanLib/Images/Image.hpp>

#include <wx/bitmap.h>
#include <wx/scrolwin.h>
#include <wx/gdicmn.h>

#include <memory>
#include <mutex>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace gs
{
  class PictureInPictureWindow : public wxScrolledWindow
  {
    public:

      PictureInPictureWindow(wxWindow* pParent);

      PictureInPictureWindow(
        wxWindow* pParent,
        const wxImage& Image1,
        const wxImage& Image2);

      void SetImage1(const dl::image::Image& image);

      void SetImage2(const dl::image::Image& image);

    private:

      void ConnectWxStuff();

      void OnPaint(wxPaintEvent& Event);

      wxPoint GetMiniWindowLocation() const;

      void OnResize(wxSizeEvent& Event);

      wxImage GeneratePrimaryImage() const;

      wxSize GetThumbnailSize() const;

      wxImage GenerateThumbnail(
        const std::optional<wxRect> portionOfTheImageToThumbnail = std::nullopt) const;

      void OnLeftClickUp(wxMouseEvent& Event);

      void OnLeftClickDown(wxMouseEvent& Event);

      void OnLeftClickDoubleClick(wxMouseEvent& Event);

      bool IsClickInMiniWindow(const wxPoint& Point);

      void OnMouseCaptureLost(wxMouseCaptureLostEvent& Event);

      void OnMouseMotion(wxMouseEvent& Event);

      void PanPrimaryImage(const wxPoint& Position);

    private:

      dl::image::Image mImage1;

      dl::image::Image mImage2;

      mutable std::mutex mImageMutex;

      bool mIsPrimaryDisplayBitmap1;

      wxPoint mSecondaryViewStart;

      mutable std::mutex mBitmapMutex;

      wxBitmap mThumbnail;

      wxBitmap mPrimaryBitmap;

      std::unique_ptr<wxPoint> mpDrag;

      wxPoint mViewStart;

      bool mIsMouseCaptured;

      static constexpr unsigned mThumbnailWidth = 340;

      static constexpr unsigned mThumbnailHeight = 220;
  };
}
