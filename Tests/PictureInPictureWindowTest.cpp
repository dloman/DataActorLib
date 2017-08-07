#include <GuiStuff/PictureInPictureWindow.hpp>
#include <wx/app.h>
#include <wx/frame.h>
#include <wx/sizer.h>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class App : public wxApp
{
  public:

    bool OnInit() override;

};

IMPLEMENT_APP(App);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool App::OnInit()
{
  wxInitAllImageHandlers();

  auto pFrame =
    new wxFrame(
      nullptr,
      wxID_ANY,
      wxT("test frame"),
      wxPoint(200, 200),
      wxSize(600, 600));

  wxBoxSizer* pSizer = new wxBoxSizer(wxHORIZONTAL);

  wxImage Image1, Image2;

  auto Image1Filename = "/home/dloman/Source/GuiStuff/Tests/Static/pic.png";
  auto Image2Filename = "/home/dloman/Source/GuiStuff/Tests/Static/pic2.png";

  if (
    !Image1.LoadFile(Image1Filename, wxBITMAP_TYPE_ANY) ||
    !Image2.LoadFile(Image2Filename , wxBITMAP_TYPE_ANY))
  {
    return false;
  }

  auto pPictureInPicture =
    new gs::PictureInPictureWindow(pFrame);

  auto pImageWrapper1 = std::make_shared<const dl::image::Image>(
    Image1.GetWidth(),
    Image1.GetHeight(),
    std::experimental::make_observer(reinterpret_cast<std::byte*>(Image1.GetData())));

  auto pImageWrapper2 = std::make_shared<const dl::image::Image>(
    Image2.GetWidth(),
    Image2.GetHeight(),
    std::experimental::make_observer(reinterpret_cast<std::byte*>(Image2.GetData())));

  pPictureInPicture->SetImage1(pImageWrapper1);

  pPictureInPicture->SetImage2(pImageWrapper2);

  pSizer->Add(pPictureInPicture, 1, wxEXPAND);

  pFrame->SetSizer(pSizer);

  pFrame->Show();

  return true;
}
