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

      void OnResize(wxSizeEvent& Event);

      void OnLeftClickUp(wxMouseEvent& Event);

      void OnLeftClickDown(wxMouseEvent& Event);

      void OnMouseCaptureLost(wxMouseCaptureLostEvent& Event);

      void OnMouseMotion(wxMouseEvent& Event);

      void PanPrimaryImage(const wxPoint& Position);

    private:

      wxBitmap mBitmap1;

      wxBitmap mBitmap2;

      wxBitmap& mPrimaryBitmap;

      wxBitmap& mSecondaryBitmap;

      wxBitmap mThumbnail;

      std::unique_ptr<wxPoint> mpDrag;

      wxPoint mViewStart;

      static constexpr int mThumbnailWidth = 340;

      static constexpr int mThumbnailHeight = 220;
  };
}
