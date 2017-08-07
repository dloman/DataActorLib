#include "PictureInPictureWindow.hpp"
#include <GuiStuff/Helpers.hpp>

#include <wx/dcbuffer.h>

#include <optional>

using gs::PictureInPictureWindow;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
PictureInPictureWindow::PictureInPictureWindow(wxWindow* pParent)
  : wxScrolledWindow(pParent, wxID_ANY),
    mpImage1(nullptr),
    mpImage2(nullptr),
    mImageMutex(),
    mIsPrimaryDisplayBitmap1(true),
    mPrimaryDisplayMutex(),
    mSecondaryViewStart(0, 0),
    mThumbnail(),
    mpDrag(nullptr),
    mViewStart(0, 0),
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
    std::lock_guard lock(mImageMutex);

    if (mPrimaryBitmap.IsOk())
    {
      Dc.DrawBitmap(mPrimaryBitmap, 0, 0, true);
    }

    if (mThumbnail.IsOk())
    {
      auto location = DoGetMiniWindowLocation();

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
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxSize PictureInPictureWindow::GetDesiredPrimaryImageSize(
  std::experimental::observer_ptr<const dl::image::Image> pImage) const
{
  auto size = GetSize();

  auto WidthScale =
    static_cast<double>(size.GetWidth()) / static_cast<double>(pImage->GetWidth());

  auto HeightScale =
    static_cast<double>(size.GetHeight()) / static_cast<double>(pImage->GetHeight());

  auto Scale = std::min(WidthScale, HeightScale);

  Scale = std::max(1.0, Scale);

  return wxSize(pImage->GetWidth() * Scale, pImage->GetHeight() * Scale);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxPoint PictureInPictureWindow::DoGetMiniWindowLocation() const
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
    std::lock_guard imageLock(mImageMutex);

    mPrimaryBitmap = DoGeneratePrimaryImage();

    SetScrollbars(
      1,
      1,
      mPrimaryBitmap.GetWidth(),
      mPrimaryBitmap.GetHeight(),
      mViewStart.x,
      mViewStart.y);
  }

  Refresh();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxImage PictureInPictureWindow::DoGeneratePrimaryImage()
{
  auto pImage = std::experimental::make_observer(mpImage1.get());

  if (!mIsPrimaryDisplayBitmap1)
  {
    pImage = std::experimental::make_observer(mpImage2.get());
  }

  if (!pImage)
  {
    return wxImage();
  }

  wxImage displayImage(
    pImage->GetWidth(),
    pImage->GetHeight(),
    reinterpret_cast<unsigned char*> (pImage->GetData().get()),
    true);

  SetScrollbars(
    1,
    1,
    displayImage.GetWidth(),
    displayImage.GetHeight(),
    mViewStart.x,
    mViewStart.y);

  auto size = GetDesiredPrimaryImageSize(pImage);

  return displayImage.Rescale(
    size.GetWidth(),
    size.GetHeight(),
    wxIMAGE_QUALITY_NEAREST);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
wxSize PictureInPictureWindow::DoGetThumbnailSize() const
{
  auto pSecondaryImage = std::experimental::make_observer(mpImage1.get());

  if (!mIsPrimaryDisplayBitmap1)
  {
    pSecondaryImage = std::experimental::make_observer(mpImage2.get());
  }

  if (!pSecondaryImage)
  {
    return wxSize();
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
wxImage PictureInPictureWindow::DoGenerateThumbnail(
  const std::optional<wxRect> portionOfTheImageToThumbnail) const
{
  auto desiredSize = DoGetThumbnailSize();
  auto pSecondaryImage = std::experimental::make_observer(mpImage2.get());

  if (!mIsPrimaryDisplayBitmap1)
  {
    pSecondaryImage = std::experimental::make_observer(mpImage1.get());
  }

  if (!pSecondaryImage)
  {
    return wxImage();
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
  if (image.GetHeight() < desiredSize.GetHeight())
  {
    desiredSize.SetHeight(image.GetHeight());
  }

  return image.Rescale(desiredSize.x, desiredSize.y, wxIMAGE_QUALITY_NEAREST);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::OnLeftClickDown(wxMouseEvent& event)
{
  if (!DoIsClickInMiniWindow(event.GetPosition()))
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
  if (DoIsClickInMiniWindow(event.GetPosition()))
  {
    auto viewStart = GetViewStart();

    {
      std::lock_guard lock(mImageMutex);

      mIsPrimaryDisplayBitmap1 = !mIsPrimaryDisplayBitmap1;

      mThumbnail = DoGenerateThumbnail();

      mPrimaryBitmap = DoGeneratePrimaryImage();
    }

    Scroll(mSecondaryViewStart);

    mSecondaryViewStart = viewStart;

    Refresh();
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool PictureInPictureWindow::DoIsClickInMiniWindow(const wxPoint& point)
{
 auto location = DoGetMiniWindowLocation() - GetViewStart();

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

  mViewStart = GetViewStart();

  Refresh();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage1(
  const std::shared_ptr<const dl::image::Image>& pImage)
{
  {
    std::lock_guard Lock(mImageMutex);

    mpImage1 = pImage;
  }

  gs::DoOnGuiThread(
    [this]
  {
    std::lock_guard lock(mImageMutex);

    if (mIsPrimaryDisplayBitmap1)
    {
      mPrimaryBitmap = DoGeneratePrimaryImage();
    }
    else
    {
      mThumbnail = DoGenerateThumbnail();;
    }

    Refresh();
  });
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void PictureInPictureWindow::SetImage2(
  const std::shared_ptr<const dl::image::Image>& pImage)
{
  {
    std::lock_guard Lock(mImageMutex);

    mpImage2 = pImage;
  }

  gs::DoOnGuiThread([this]
  {
    std::lock_guard Lock(mImageMutex);

    if (!mIsPrimaryDisplayBitmap1)
    {
      mPrimaryBitmap = DoGeneratePrimaryImage();
    }
    else
    {
      mThumbnail = DoGenerateThumbnail();;
    }

    Refresh();
  });
}
