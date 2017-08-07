#pragma once

#include <DanLib/Images/Image.hpp>

#include <wx/bitmap.h>
#include <wx/scrolwin.h>
#include <wx/gdicmn.h>

#include <memory>
#include <experimental/memory>
#include <mutex>
#include <iostream>

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

      void SetImage1(const std::shared_ptr<const dl::image::Image>& pImage);

      void SetImage2(const std::shared_ptr<const dl::image::Image>& pImage);

    private:

      void ConnectWxStuff();

      void OnPaint(wxPaintEvent& Event);

      wxPoint DoGetMiniWindowLocation() const;

      void OnResize(wxSizeEvent& Event);

      wxImage DoGeneratePrimaryImage();

      wxSize DoGetThumbnailSize() const;

      wxImage DoGenerateThumbnail(
        const std::optional<wxRect> portionOfTheImageToThumbnail = std::nullopt) const;

      void OnLeftClickUp(wxMouseEvent& Event);

      void OnLeftClickDown(wxMouseEvent& Event);

      void OnLeftClickDoubleClick(wxMouseEvent& Event);

      bool DoIsClickInMiniWindow(const wxPoint& Point);

      void OnMouseCaptureLost(wxMouseCaptureLostEvent& Event);

      void OnMouseMotion(wxMouseEvent& Event);

      void PanPrimaryImage(const wxPoint& Position);

      wxSize GetDesiredPrimaryImageSize(
        std::experimental::observer_ptr<const dl::image::Image> pImage) const;

    private:

      std::shared_ptr<const dl::image::Image> mpImage1;

      std::shared_ptr<const dl::image::Image> mpImage2;

      mutable std::mutex mImageMutex;

      bool mIsPrimaryDisplayBitmap1;

      mutable std::mutex mPrimaryDisplayMutex;

      wxPoint mSecondaryViewStart;

      wxBitmap mThumbnail;

      wxBitmap mPrimaryBitmap;

      std::unique_ptr<wxPoint> mpDrag;

      wxPoint mViewStart;

      bool mIsMouseCaptured;

      static constexpr unsigned mThumbnailWidth = 340;

      static constexpr unsigned mThumbnailHeight = 220;
  };
}
