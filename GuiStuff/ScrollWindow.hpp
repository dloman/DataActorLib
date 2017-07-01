#pragma once

#include <memory>

#include <wx/bitmap.h>
#include <wx/scrolwin.h>
#include <wx/gdicmn.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace gs
{
  class ScrollWindow : public wxScrolledWindow
  {
    public:

      ScrollWindow(wxWindow* pParent, const wxImage& Image);

    private:

      void ConnectWxStuff();

      void OnPaint(wxPaintEvent& Event);

      void OnLeftClickUp(wxMouseEvent& Event);

      void OnLeftClickDown(wxMouseEvent& Event);

      void OnMouseCaptureLost(wxMouseCaptureLostEvent& Event);

      void OnMouseMotion(wxMouseEvent& Event);

      void PanImage(const wxPoint& Position);

    private:

      wxBitmap mBitmap;

      std::unique_ptr<wxPoint> mpDrag;

      wxPoint mViewStart;
  };
}
