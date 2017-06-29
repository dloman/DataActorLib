#include "Packets.hpp"

#include <GuiStuff/GridDisplayer.hpp>

#include <wx/app.h>
#include <wx/sizer.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <random>
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

  mIsRunning = true;

  mpThread.reset(new std::thread([pGridDisplayer, this]
    {
      auto Random = []
      {
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<uint8_t> distribution;
        return distribution(generator);
      };

      while (mIsRunning)
      {
        dal::test::MotorCommand MotorCommand{Random(), Random(), Random()};

        pGridDisplayer->Set(MotorCommand);

        dal::test::Position Position{
        static_cast<double>(Random()),
        static_cast<double>(Random()),
        static_cast<double>(Random()),
        static_cast<double>(std::chrono::system_clock::now().time_since_epoch().count())};

        pGridDisplayer->Set(Position);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
      }
    }));

  return true;
}
