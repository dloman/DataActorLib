
#include <Utility/Helpers.hpp>
#include <wx/app.h>
#include <wx/sizer.h>
#include <wx/frame.h>
#include <future>
#include <iostream>
#include <thread>
#include <utility>

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

  SetAppName("Do On Wx Thread Test");

  auto pFrame = new wxFrame(nullptr, wxID_ANY, "Do On Wx Thread Test");

  using namespace std;
  cout << "Main Thread Id = " << this_thread::get_id() << endl;

  std::async(
    std::launch::async,
    [&]
    {
      cout << "Worker Thread Id = " << this_thread::get_id() << endl;

      gs::DoOnGuiThread(
        [&] { cout << "this should be main id = " << this_thread::get_id() << endl;});
    });

  pFrame->Layout();

  pFrame->Show();

  return true;
}
