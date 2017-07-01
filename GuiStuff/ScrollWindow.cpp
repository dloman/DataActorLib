#include "ScrollWindow.hpp"

#include <wx/dcclient.h>

using gs::ScrollWindow;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ScrollWindow::ScrollWindow(wxWindow* pParent, const wxImage& Image)
  : wxScrolledWindow(pParent, wxID_ANY),
    mBitmap(Image),
    mpDrag(nullptr),
    mViewStart()
{
   SetScrollbars(1,1,800,800, 0, 0);

   Refresh();
   Update();
   ConnectWxStuff();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ScrollWindow::ConnectWxStuff()
{
  Bind(wxEVT_LEFT_DOWN, &ScrollWindow::OnLeftClickDown, this);
  Bind(wxEVT_LEFT_UP, &ScrollWindow::OnLeftClickUp, this);
  Bind(wxEVT_MOTION, &ScrollWindow::OnMouseMotion, this);
  Bind(wxEVT_MOUSE_CAPTURE_LOST, &ScrollWindow::OnMouseCaptureLost, this);
  Bind(wxEVT_PAINT, &ScrollWindow::OnPaint, this);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ScrollWindow::OnPaint(wxPaintEvent& Event)
{
  wxPaintDC Dc(this);
  DoPrepareDC(Dc);
  Dc.DrawBitmap(mBitmap, 0, 0, true);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ScrollWindow::OnLeftClickDown(wxMouseEvent& Event)
{
  mViewStart = GetViewStart();
  mpDrag.reset(new wxPoint(Event.GetPosition()));
  CaptureMouse();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ScrollWindow::OnMouseMotion(wxMouseEvent& Event)
{
  if (mpDrag)
  {
    PanImage(Event.GetPosition());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ScrollWindow::OnLeftClickUp(wxMouseEvent& Event)
{
  if (mpDrag)
  {
    PanImage(Event.GetPosition());
    mpDrag.reset(nullptr);
  }
  ReleaseMouse();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ScrollWindow::OnMouseCaptureLost(
  wxMouseCaptureLostEvent& Event)
{
  ReleaseMouse();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void ScrollWindow::PanImage(const wxPoint& Position)
{
  auto ScrollDistance = ((*mpDrag - Position));

  Scroll(mViewStart + ScrollDistance);

  Refresh();
}
