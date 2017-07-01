#include "Packets.hpp"

#include <GuiStuff/GridDisplayer.hpp>

#include <Random/Random.hpp>

#include <wx/app.h>
#include <wx/sizer.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

//******************************************************************************
//******************************************************************************
class App : public wxApp
{
  public:

    ~App()
    {
      mIsRunning = false;

      if (mpThread && mpThread->joinable())
      {
        mpThread->join();
      }
    }

    bool OnInit() override;

  private:

    std::atomic<bool> mIsRunning;

    std::unique_ptr<std::thread> mpThread;
};

IMPLEMENT_APP(App);

namespace
{

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  gs::test::MotorCommand GetRandomMotorCommand()
  {
    return
    {
      dl::random::GetUniform<uint8_t>(),
      dl::random::GetUniform<uint8_t>(),
      dl::random::GetUniform<uint8_t>()
    };
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  gs::test::Position GetRandomPosition()
  {
    return
    {
      dl::random::GetUniform<double>(),
      dl::random::GetUniform<double>(),
      dl::random::GetUniform<double>(),
      static_cast<double>(std::chrono::system_clock::now().time_since_epoch().count())
    };
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool App::OnInit()
{
  SetVendorName("Lomancer Heavy Industries");
  SetAppName("Grid Displayer Test");

  auto pFrame = new wxFrame(nullptr, wxID_ANY, "Grid Displayer Test");

  auto pGridDisplayer =
    new gs::GridDisplayer<gs::test::MotorCommand, gs::test::Position>(pFrame);

  auto pMainSizer = new wxBoxSizer(wxHORIZONTAL);

  pMainSizer->Add(pGridDisplayer, 1, wxEXPAND | wxALL, 5);

  pFrame->SetSizer(pMainSizer);

  pFrame->Layout();

  pFrame->Show();

  mIsRunning = true;

  mpThread.reset(new std::thread([pGridDisplayer, this]
    {
      while (mIsRunning)
      {
        auto MotorCommand = GetRandomMotorCommand();

        pGridDisplayer->Set(MotorCommand);

        auto Position = GetRandomPosition();

        pGridDisplayer->Set(Position);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
    }));

  return true;
}
