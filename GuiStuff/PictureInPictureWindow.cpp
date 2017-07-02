#include "PictureInPictureWindow.hpp"

#include <wx/dcbuffer.h>

#include <optional>

using gs::PictureInPictureWindow;

namespace
{
  wxBitmap GenerateThumbnail(
    const wxImage& originalImage,
    const wxSize desiredSize,
    const std::optional<wxRect> portionOfTheImageToThumbnail = std::nullopt)
  {
    wxImage image;

    if (portionOfTheImageToThumbnail)
    {
      image = originalImage.GetSubImage(*portionOfTheImageToThumbnail);
    }
    else
    {
      image = originalImage;
    }
    auto scaleFactor =
      static_cast<double>(desiredSize.GetWidth()) / image.GetWidth();

    image.Rescale(desiredSize.GetWidth(), image.GetHeight() * scaleFactor);

    return image.Resize(desiredSize, wxPoint(0, 0));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PictureInPictureWindow::PictureInPictureWindow(
  wxWindow* pParent,
  const wxImage& image1,
  const wxImage& image2)
  : wxScrolledWindow(pParent, wxID_ANY),
    mBitmap1(image1),
    mBitmap2(image2),
    mPrimaryBitmap(mBitmap1),
    mSecondaryBitmap(mBitmap2),
    mSecondaryViewStart(0, 0),
    mThumbnail(GenerateThumbnail(image2, wxSize(mThumbnailWidth, mThumbnailHeight))),
    mpDrag(nullptr),
    mViewStart(),
    mIsMouseCaptured(false)
{
   auto size = mPrimaryBitmap.GetSize();

   SetScrollbars(1, 1, size.GetWidth(), size.GetHeight(), 0, 0);

   Refresh();

   Update();

   ConnectWxStuff();

   SetDoubleBuffered(true);

   ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::ConnectWxStuff()
{
  Bind(wxEVT_LEFT_DOWN, &PictureInPictureWindow::OnLeftClickDown, this);
  Bind(wxEVT_LEFT_DCLICK, &PictureInPictureWindow::OnLeftClickDoubleClick, this);
  Bind(wxEVT_LEFT_UP, &PictureInPictureWindow::OnLeftClickUp, this);
  Bind(wxEVT_MOTION, &PictureInPictureWindow::OnMouseMotion, this);
  Bind(wxEVT_MOUSE_CAPTURE_LOST, &PictureInPictureWindow::OnMouseCaptureLost, this);
  Bind(wxEVT_PAINT, &PictureInPictureWindow::OnPaint, this);
  Bind(wxEVT_SIZE, &PictureInPictureWindow::OnResize, this);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnPaint(wxPaintEvent& Event)
{
  wxBufferedPaintDC Dc(this);
  DoPrepareDC(Dc);

  Dc.DrawBitmap(mPrimaryBitmap, 0, 0, true);

  auto Location = GetMiniWindowLocation();

  Dc.SetBrush(*wxBLACK_BRUSH);

  Dc.DrawRectangle(Location.x - 2, Location.y -2, mThumbnailWidth + 4, mThumbnailHeight + 4);

  Dc.DrawBitmap(mThumbnail, Location.x, Location.y, true);

  Event.Skip();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxPoint PictureInPictureWindow::GetMiniWindowLocation() const
{
  auto Location = GetViewStart();

  auto size = GetSize();

  Location.x += .03 * size.GetWidth();

  Location.y += .63 * size.GetHeight();

  return Location;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnResize(wxSizeEvent& Event)
{
  Refresh();
  Update();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnLeftClickDown(wxMouseEvent& event)
{
  if (!IsClickInMiniWindow(event.GetPosition()))
  {
    mViewStart = GetViewStart();

    mpDrag.reset(new wxPoint(event.GetPosition()));

    mIsMouseCaptured = true;

    CaptureMouse();
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnLeftClickDoubleClick(wxMouseEvent& event)
{
  if (IsClickInMiniWindow(event.GetPosition()))
  {
    auto viewStart = GetViewStart();

    mThumbnail = GenerateThumbnail(
        mPrimaryBitmap.ConvertToImage(),
        wxSize(mThumbnailWidth, mThumbnailHeight),
        wxRect(viewStart, GetSize()));

    std::swap(mPrimaryBitmap, mSecondaryBitmap);

    Scroll(mSecondaryViewStart);

    mSecondaryViewStart = viewStart;

    Refresh();
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool PictureInPictureWindow::IsClickInMiniWindow(const wxPoint& point)
{
 auto location = GetMiniWindowLocation() - GetViewStart();

 if (
   point.x > location.x && point.x < location.x + mThumbnailWidth &&
   point.y > location.y && point.y < location.y + mThumbnailHeight)
 {
   return true;
 }
 return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnMouseMotion(wxMouseEvent& Event)
{
  if (mpDrag)
  {
    PanPrimaryImage(Event.GetPosition());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnLeftClickUp(wxMouseEvent& Event)
{
  if (mpDrag)
  {
    PanPrimaryImage(Event.GetPosition());
    mpDrag.reset(nullptr);
  }
  if (mIsMouseCaptured)
  {
    ReleaseMouse();

    mIsMouseCaptured = false;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnMouseCaptureLost(
  wxMouseCaptureLostEvent& Event)
{
  if (mIsMouseCaptured)
  {
    ReleaseMouse();

    mIsMouseCaptured = false;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::PanPrimaryImage(const wxPoint& Position)
{
  auto ScrollDistance = ((*mpDrag - Position));

  Scroll(mViewStart + ScrollDistance);

  Refresh();
}
