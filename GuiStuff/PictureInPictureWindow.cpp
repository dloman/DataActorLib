#include "PictureInPictureWindow.hpp"
#include <GuiStuff/Helpers.hpp>

#include <wx/dcbuffer.h>

#include <optional>

using gs::PictureInPictureWindow;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PictureInPictureWindow::PictureInPictureWindow(wxWindow* pParent)
  : wxScrolledWindow(pParent, wxID_ANY),
    mImage1(),
    mImage2(),
    mImageMutex(),
    mIsPrimaryDisplayBitmap1(true),
    mSecondaryViewStart(0, 0),
    mBitmapMutex(),
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
void PictureInPictureWindow::OnPaint(wxPaintEvent& event)
{
  wxBufferedPaintDC Dc(this);
  DoPrepareDC(Dc);

  Dc.Clear();

  {
    std::lock_guard lock(mBitmapMutex);

    if (mPrimaryBitmap.IsOk())
    {
      Dc.DrawBitmap(mPrimaryBitmap, 0, 0, true);
    }

    if (mThumbnail.IsOk())
    {
      auto location = GetMiniWindowLocation();

      Dc.SetBrush(*wxBLACK_BRUSH);

      auto thumbnailSize = mThumbnail.GetSize();

      Dc.DrawRectangle(
        location.x - 2,
        location.y - 2,
        thumbnailSize.GetWidth() + 4,
        thumbnailSize.GetHeight() + 4);

        Dc.DrawBitmap(mThumbnail, location.x, location.y, true);
    }
  }
  event.Skip();
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
  {
    std::lock_guard lock(mBitmapMutex);

    mPrimaryBitmap = GeneratePrimaryImage();
  }

  Refresh();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxImage PictureInPictureWindow::GeneratePrimaryImage() const
{
  auto pImage= &mImage1;

  if (!mIsPrimaryDisplayBitmap1)
  {
    pImage= &mImage2;
  }

  wxImage displayImage(
    pImage->GetWidth(),
    pImage->GetHeight(),
    reinterpret_cast<unsigned char*> (pImage->GetData().get()),
    true);

  return displayImage.Resize(GetSize(), wxPoint(0, 0));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxSize PictureInPictureWindow::GetThumbnailSize() const
{
  auto pSecondaryImage = &mImage1;

  if (!mIsPrimaryDisplayBitmap1)
  {
    pSecondaryImage = &mImage2;
  }

  auto width = mThumbnailWidth;

  if (width > pSecondaryImage->GetWidth())
  {
    width = pSecondaryImage->GetWidth();
  }

  auto height = mThumbnailHeight;

  if (height > pSecondaryImage->GetHeight())
  {
    height = pSecondaryImage->GetHeight();
  }

  return wxSize(width, height);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxImage PictureInPictureWindow::GenerateThumbnail(
  const std::optional<wxRect> portionOfTheImageToThumbnail) const
{
  auto desiredSize = GetThumbnailSize();
  auto pSecondaryImage = &mImage2;

  if (!mIsPrimaryDisplayBitmap1)
  {
    pSecondaryImage = &mImage1;
  }

  wxImage originalWxImage(
    pSecondaryImage->GetWidth(),
    pSecondaryImage->GetHeight(),
    reinterpret_cast<unsigned char*> (pSecondaryImage->GetData().get()),
    true);

  auto originalSize = originalWxImage.GetSize();

  wxImage image;

  if (portionOfTheImageToThumbnail)
  {
    auto subImageSize = portionOfTheImageToThumbnail->GetSize();


    if (
      subImageSize.GetWidth() < originalSize.GetWidth() &&
      subImageSize.GetHeight() < originalSize.GetHeight())
    {
      image = originalWxImage.GetSubImage(*portionOfTheImageToThumbnail);
    }
    else
    {
      image = originalWxImage;
    }
  }
  else
  {
    image = originalWxImage;
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

    {
      std::lock_guard imageLock(mImageMutex);

      auto pPrimaryImage = &mImage1;
      auto pSecondaryImage = &mImage2;

      if (!mIsPrimaryDisplayBitmap1)
      {
        std::swap(pPrimaryImage, pSecondaryImage);
      }

      std::lock_guard bitmapLock(mBitmapMutex);

      mIsPrimaryDisplayBitmap1 = !mIsPrimaryDisplayBitmap1;

      mThumbnail = GenerateThumbnail();

      mPrimaryBitmap = GeneratePrimaryImage();
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
   static_cast<int>(point.x) > location.x &&
   static_cast<unsigned>(point.x) < location.x + mThumbnailWidth &&
   static_cast<int>(point.y) > location.y &&
   static_cast<unsigned>(point.y) < location.y + mThumbnailHeight)
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
void PictureInPictureWindow::SetImage1(const dl::image::Image& image)
{
  {
    std::lock_guard Lock(mImageMutex);

    mImage1 = image;
  }

  std::optional<wxImage> primaryImage, thumbnail;

  {
    std::lock_guard Lock(mBitmapMutex);

    if (mIsPrimaryDisplayBitmap1)
    {
      primaryImage = GeneratePrimaryImage();
    }
    else
    {
      thumbnail = GenerateThumbnail();
    }
  }

  gs::DoOnGuiThread(
    [this, primaryImage = std::move(primaryImage), thumbnail = std::move(thumbnail)]
  {
    std::lock_guard Lock(mBitmapMutex);

    if (primaryImage)
    {
      mPrimaryBitmap = *primaryImage;
    }
    else
    {
      mThumbnail = *thumbnail;
    }

    Refresh();
  });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage2(const dl::image::Image& image)
{
  {
    std::lock_guard Lock(mImageMutex);

    mImage2 = image;
  }

  std::optional<wxImage> primaryImage, thumbnail;

  {
    std::lock_guard Lock(mBitmapMutex);

    if (!mIsPrimaryDisplayBitmap1)
    {
      primaryImage = GeneratePrimaryImage();
    }
    else
    {
      thumbnail = GenerateThumbnail();
    }
  }

  gs::DoOnGuiThread(
    [this, primaryImage = std::move(primaryImage), thumbnail = std::move(thumbnail)]
  {
    std::lock_guard Lock(mBitmapMutex);

    if (primaryImage)
    {
      mPrimaryBitmap = *primaryImage;
    }
    else
    {
      mThumbnail = *thumbnail;
    }

    Refresh();
  });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage1(dl::image::Image&& image)
{
  {
    std::lock_guard Lock(mImageMutex);

    mImage1 = std::move(image);
  }

  std::optional<wxImage> primaryImage, thumbnail;

  {
    std::lock_guard Lock(mBitmapMutex);

    if (mIsPrimaryDisplayBitmap1)
    {
      primaryImage = GeneratePrimaryImage();
    }
    else
    {
      thumbnail = GenerateThumbnail();
    }
  }

  gs::DoOnGuiThread(
    [this, primaryImage = std::move(primaryImage), thumbnail = std::move(thumbnail)]
  {
    std::lock_guard Lock(mBitmapMutex);

    if (primaryImage)
    {
      mPrimaryBitmap = *primaryImage;
    }
    else
    {
      mThumbnail = *thumbnail;
    }

    Refresh();
  });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage2(dl::image::Image&& image)
{
  {
    std::lock_guard Lock(mImageMutex);

    mImage2 = std::move(image);
  }

  std::optional<wxImage> primaryImage, thumbnail;

  {
    std::lock_guard Lock(mBitmapMutex);

    if (!mIsPrimaryDisplayBitmap1)
    {
      primaryImage = GeneratePrimaryImage();
    }
    else
    {
      thumbnail = GenerateThumbnail();
    }
  }

  gs::DoOnGuiThread(
    [this, primaryImage = std::move(primaryImage), thumbnail = std::move(thumbnail)]
  {
    std::lock_guard Lock(mBitmapMutex);

    if (primaryImage)
    {
      mPrimaryBitmap = *primaryImage;
    }
    else
    {
      mThumbnail = *thumbnail;
    }

    Refresh();
  });
}
