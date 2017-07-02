#pragma once

#include <memory>

#include <wx/bitmap.h>
#include <wx/scrolwin.h>
#include <wx/gdicmn.h>

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

      wxBitmap mThumbnail;

      std::unique_ptr<wxPoint> mpDrag;

      wxPoint mViewStart;

      bool mIsMouseCaptured;

      static constexpr int mThumbnailWidth = 340;

      static constexpr int mThumbnailHeight = 220;
  };
}
