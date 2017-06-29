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

  auto pMainSizer = new wxBoxSizer(wxHORIZONTAL);

  pMainSizer->Add(pGridDisplayer, 1, wxEXPAND | wxALL, 5);

  pFrame->SetSizer(pMainSizer);

  pFrame->Layout();

  pFrame->Show();

  dal::test::MotorCommand MotorCommand{1, 2, 3};

  pGridDisplayer->Set(MotorCommand);

  pGridDisplayer->Refresh();

  dal::test::Position Position{4, 5, 6};

  pGridDisplayer->Set(Position);

  pGridDisplayer->Refresh();
  return true;
}
