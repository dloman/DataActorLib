#pragma once

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

      PictureInPictureWindow(
        wxWindow* pParent,
        const wxImage& Image1,
        const wxImage& Image2);

      void SetImage1(const wxImage& bitmap);

      void SetImage2(const wxImage& bitmap);

    private:

      void ConnectWxStuff();

      void OnPaint(wxPaintEvent& Event);

      wxPoint GetMiniWindowLocation() const;

      void OnResize(wxSizeEvent& Event);

      void OnLeftClickUp(wxMouseEvent& Event);

      void OnLeftClickDown(wxMouseEvent& Event);

      void OnLeftClickDoubleClick(wxMouseEvent& Event);

      bool IsClickInMiniWindow(const wxPoint& Point);

      void OnMouseCaptureLost(wxMouseCaptureLostEvent& Event);

      void OnMouseMotion(wxMouseEvent& Event);

      void PanPrimaryImage(const wxPoint& Position);

    private:

      wxBitmap mBitmap1;

      wxBitmap mBitmap2;

      wxBitmap& mPrimaryBitmap;

      wxBitmap& mSecondaryBitmap;

      wxPoint mSecondaryViewStart;

      mutable std::mutex mImageMutex;

      wxBitmap mThumbnail;

      std::unique_ptr<wxPoint> mpDrag;

      wxPoint mViewStart;

      bool mIsMouseCaptured;

      static constexpr int mThumbnailWidth = 340;

      static constexpr int mThumbnailHeight = 220;
  };
}
