#include "PictureInPictureWindow.hpp"
#include <GuiStuff/Helpers.hpp>

#include <wx/dcbuffer.h>

#include <optional>

using gs::PictureInPictureWindow;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace
{
  wxBitmap GenerateThumbnail(
    const wxImage& originalImage,
    wxSize desiredSize,
    const std::optional<wxRect> portionOfTheImageToThumbnail = std::nullopt)
  {
    wxImage image;

    auto originalSize = originalImage.GetSize();

    if (portionOfTheImageToThumbnail)
    {
      auto subImageSize = portionOfTheImageToThumbnail->GetSize();

      if (
        subImageSize.GetWidth() < originalSize.GetWidth() &&
        subImageSize.GetHeight() < originalSize.GetHeight())
      {
        image = originalImage.GetSubImage(*portionOfTheImageToThumbnail);
      }
      else
      {
        image = originalImage;
      }
    }
    else
    {
      image = originalImage;
    }
    auto scaleFactor =
      static_cast<double>(desiredSize.GetWidth()) / image.GetWidth();

    image.Rescale(desiredSize.GetWidth(), image.GetHeight() * scaleFactor);

    if (image.GetHeight() < desiredSize.GetHeight())
    {
      desiredSize.SetHeight(image.GetHeight());
    }

    return image.Resize(desiredSize, wxPoint(0, 0));
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PictureInPictureWindow::PictureInPictureWindow(wxWindow* pParent)
  : wxScrolledWindow(pParent, wxID_ANY),
    mBitmap1(),
    mBitmap2(),
    mImage1(),
    mImage2(),
    mpPrimaryBitmap(&mBitmap1),
    mpSecondaryBitmap(&mBitmap2),
    mSecondaryViewStart(0, 0),
    mImageMutex(),
    mThumbnail(),
    mpDrag(nullptr),
    mViewStart(),
    mIsMouseCaptured(false)
{
   Refresh();

   ConnectWxStuff();

   SetDoubleBuffered(true);

   ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_NEVER);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PictureInPictureWindow::PictureInPictureWindow(
  wxWindow* pParent,
  const wxImage& image1,
  const wxImage& image2)
  : PictureInPictureWindow(pParent)
{
   if (image1.IsOk())
   {
     mBitmap1 = image1;

     auto size = GetSize();

     SetScrollbars(1, 1, size.GetWidth(), size.GetHeight(), 0, 0);

   }

   if (image2.IsOk())
   {
     mBitmap2 = image2;

     mThumbnail =
       GenerateThumbnail(image2, wxSize(mThumbnailWidth, mThumbnailHeight));
   }

   Refresh();
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

  Dc.Clear();

  {
    std::lock_guard lock(mImageMutex);

    if (mpPrimaryBitmap->IsOk())
    {
      Dc.DrawBitmap(*mpPrimaryBitmap, 0, 0, true);
    }

    if (mpSecondaryBitmap->IsOk() && mThumbnail.IsOk())
    {
      auto Location = GetMiniWindowLocation();

      Dc.SetBrush(*wxBLACK_BRUSH);

      auto thumbnailSize = mThumbnail.GetSize();

      Dc.DrawRectangle(
        Location.x - 2,
        Location.y - 2,
        thumbnailSize.GetWidth() + 4,
        thumbnailSize.GetHeight() + 4);

      Dc.DrawBitmap(mThumbnail, Location.x, Location.y, true);
    }
  }
  Event.Skip();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxPoint PictureInPictureWindow::GetMiniWindowLocation() const
{
  auto Location = GetViewStart();

  auto size = GetSize();

  Location.x += .03 * size.GetWidth();

  Location.y += .75 * size.GetHeight();

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
#include <iostream>
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnLeftClickDoubleClick(wxMouseEvent& event)
{
  if (IsClickInMiniWindow(event.GetPosition()))
  {
    auto viewStart = GetViewStart();

    {
      std::lock_guard lock(mImageMutex);

      if (mpPrimaryBitmap->IsOk())
      {
        mThumbnail = GenerateThumbnail(
          mpPrimaryBitmap->ConvertToImage(),
          wxSize(mThumbnailWidth, mThumbnailHeight),
          wxRect(viewStart, GetSize()));
      }

      std::swap(mpPrimaryBitmap, mpSecondaryBitmap);
    }

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
void PictureInPictureWindow::OnMouseMotion(wxMouseEvent& event)
{
  if (mpDrag)
  {
    PanPrimaryImage(event.GetPosition());
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnLeftClickUp(wxMouseEvent& event)
{
  if (mpDrag)
  {
    PanPrimaryImage(event.GetPosition());
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
void PictureInPictureWindow::PanPrimaryImage(const wxPoint& position)
{
  auto scrollDistance = ((*mpDrag - position));

  Scroll(mViewStart + scrollDistance);

  Refresh();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage1(const wxImage& image)
{
  gs::DoOnGuiThread([this, image]
  {
    {
      std::lock_guard Lock(mImageMutex);

      mBitmap1 = image;

      if (&mBitmap1 == mpSecondaryBitmap.get())
      {
        mThumbnail = GenerateThumbnail(
          mpSecondaryBitmap->ConvertToImage(),
          wxSize(mThumbnailWidth, mThumbnailHeight));
      }
    }
    Refresh();
  });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage2(const wxImage& image)
{
  gs::DoOnGuiThread([this, image]
  {
    {
      std::lock_guard Lock(mImageMutex);

      mBitmap2 = image;

      if (&mBitmap2 == mpSecondaryBitmap.get())
      {
        mThumbnail = GenerateThumbnail(
          mpSecondaryBitmap->ConvertToImage(),
          wxSize(mThumbnailWidth, mThumbnailHeight));
      }
    }
    Refresh();
  });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage1(const dl::image::Image& image)
{
  gs::DoOnGuiThread([this, image]
  {
    {
      std::lock_guard Lock(mImageMutex);

      mImage1 = image;

      wxImage displayImage(
        image.GetWidth(),
        image.GetHeight(),
        reinterpret_cast<unsigned char*> (mImage1.GetData().get()),
        true);

      mBitmap1 = displayImage;

      if (&mBitmap1 == mpSecondaryBitmap.get())
      {
        mThumbnail = GenerateThumbnail(
          mpSecondaryBitmap->ConvertToImage(),
          wxSize(mThumbnailWidth, mThumbnailHeight),
          wxRect(GetViewStart(), GetSize()));
      }
    }
    Refresh();
  });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage2(const dl::image::Image& image)
{
  gs::DoOnGuiThread([this, image]
  {
    {
      std::lock_guard Lock(mImageMutex);

      mImage2 = image;

      wxImage displayImage(
        image.GetWidth(),
        image.GetHeight(),
        reinterpret_cast<unsigned char*> (mImage2.GetData().get()),
        true);

      mBitmap2 = displayImage;

      std::cout << &mBitmap2 << " vs " << mpSecondaryBitmap.get() << std::endl;
      if (&mBitmap2 == mpSecondaryBitmap.get())
      {
        mThumbnail = GenerateThumbnail(
          mpSecondaryBitmap->ConvertToImage(),
          wxSize(mThumbnailWidth, mThumbnailHeight),
          wxRect(GetViewStart(), GetSize()));
      }
    }
    Refresh();
  });
}
