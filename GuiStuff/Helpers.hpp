#pragma once

#include <wx/app.h>
#include <wx/window.h>
#include <iostream>
namespace gs
{
  template <typename T>
  void DoOnGuiThread(T&& function)
  {
    if (wxTheApp)
    {
      wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(
        std::forward<T>(function));
    }
  }
}
