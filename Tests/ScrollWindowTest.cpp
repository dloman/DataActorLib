#include <GuiStuff/ScrollWindow.hpp>
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

  wxImage Image;

  if (
    !Image.LoadFile("/home/dloman/Source/GuiStuff/pic.png", wxBITMAP_TYPE_ANY))
  {
    return false;
  }

  auto pScrollWindow = new gs::ScrollWindow(pFrame, Image);

  pSizer->Add(pScrollWindow, 1, wxEXPAND);

  pFrame->SetSizer(pSizer);

  pFrame->Show();

  return true;
}
