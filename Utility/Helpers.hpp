#pragma once

#include <wx/app.h>
#include <wx/window.h>
namespace gs
{
  template <typename T>
  void DoOnGuiThread(T&& function)
  {
    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(
      std::forward<T>(function));
  }
}
