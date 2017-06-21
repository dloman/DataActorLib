#include "Packets.hpp"

#include <DataActorLib/GridDisplayer.hpp>
#include <wx/app.h>
#include <wx/sizer.h>

//******************************************************************************
//******************************************************************************
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
  SetVendorName("Lomancer Heavy Industries");
  SetAppName("Grid Displayer Test");

  auto pFrame = new wxFrame(nullptr, wxID_ANY, "Grid Displayer Test");

  auto pGridDisplayer =
    new dal::GridDisplayer<dal::test::MotorCommand, dal::test::Position>(pFrame);

  auto pMainOuterSizer = new wxBoxSizer(wxVERTICAL);

  auto pMainInnerSizer = new wxBoxSizer(wxHORIZONTAL);

  pMainInnerSizer->Add(pGridDisplayer, 1, wxEXPAND | wxALL, 5);

  pMainOuterSizer->Add(pMainInnerSizer, 1, wxEXPAND | wxALL, 5);

  pFrame->SetSizer(pMainOuterSizer);

  pFrame->Layout();

  pFrame->Show();

  return true;
}
