#pragma once

#include <wx/dataview.h>
#include <wx/grid.h>
#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include <boost/hana.hpp>
#include <boost/type_index.hpp>
#include <iostream>
#include <locale>

class wxWindow;

namespace dal
{
  namespace
  {
    void ConvertCamelCaseToSpaces(std::string& name)
    {
      for (size_t i = 1; i < name.size(); ++i)
      {
        if (std::isupper(name[i]) || std::isdigit(name[i]))
        {
          name.insert(i, 1, ' ');
          ++i;
        }
      }
    }

    void RemoveNamespace(std::string& name)
    {
      auto iName = name.rfind("::");

      if (iName != std::string::npos)
      {
        name = name.substr(iName + 2);
      }

      ConvertCamelCaseToSpaces(name);
    }

    template<typename PacketType>
    void AddGridLabels(wxGrid* pGrid, PacketType packet)
    {
      namespace hana = boost::hana;
      std::vector<std::string> labels;
      hana::for_each(packet, [&labels](auto pair)
      {
        labels.emplace_back(hana::to<const char*>(hana::first(pair)));
      });

      pGrid->CreateGrid(labels.size(), 1);
      for (auto i = 0u; i < labels.size(); ++i)
      {
        //remove 'm' prefix
        if (labels[i][0] == 'm')
        {
          labels[i] = labels[i].substr(1);
        }

        ConvertCamelCaseToSpaces(labels[i]);
        pGrid->SetRowLabelValue(i, labels[i]);
        pGrid->AutoSizeRowLabelSize(i);
      }
    }

    template<std::size_t Index = 0, typename TupleType>
      typename std::enable_if_t<Index == std::tuple_size<TupleType>::value>
      AddPages(wxNotebook* pNotebook, const TupleType& tuple)
      {
      }

    template<std::size_t Index = 0, typename TupleType>
      typename std::enable_if_t<Index != std::tuple_size<TupleType>::value>
      AddPages(wxNotebook* pNotebook, const TupleType& tuple)
    {
      using PacketType = decltype(std::get<Index>(tuple));

      auto name = boost::typeindex::type_id<PacketType>().pretty_name();

      RemoveNamespace(name);

      auto pPage = new wxPanel(pNotebook, wxID_ANY);

      auto pGrid = new wxGrid(pPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

      // Grid
      AddGridLabels(pGrid, std::get<Index>(tuple));

      pGrid->EnableEditing(false);
      pGrid->EnableGridLines(true);
      pGrid->EnableDragGridSize();
      pGrid->SetMargins(0, 0);

      // Columns
      pGrid->EnableDragColMove(false);
      pGrid->EnableDragColSize(false);
      pGrid->SetColLabelSize(0);
      pGrid->AutoSizeColumns();
      pGrid->HideColLabels();

      // Rows
      pGrid->EnableDragRowSize(false);
      pGrid->AutoSizeRows();

      // Cell Defaults
      pGrid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);

      auto pPageSizer = new wxBoxSizer(wxHORIZONTAL);

      auto Flags = wxSizerFlags(1).Center().Expand().Border(wxALL, 5);

      pPageSizer->Add(pGrid, Flags);

      pPage->SetSizer(pPageSizer);
      pPage->Layout();
      pPageSizer->Fit(pPage);

      pNotebook->AddPage(pPage, name, false);

      AddPages<Index + 1>(pNotebook, tuple);
    }
  }

  template <typename ... Args>
    class GridDisplayer : public wxPanel
  {
    public:

      GridDisplayer(wxWindow* pParent)
        : wxPanel(pParent, wxID_ANY),
        mFields()
    {
      SetSizeHints(wxDefaultSize, wxDefaultSize);

      auto pPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);

      auto pMainSizer = new wxBoxSizer(wxHORIZONTAL);

      auto pNotebook = new wxNotebook(pPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

      AddPages(pNotebook, mFields);

      pMainSizer->Add(pNotebook, 1, wxEXPAND | wxALL, 5);

      pPanel->SetSizer(pMainSizer);
      pPanel->Layout();
      pMainSizer->Fit(pPanel);

      auto pFrameSizer = new wxBoxSizer(wxHORIZONTAL);
      pFrameSizer->Add(pPanel, 1, wxEXPAND | wxALL, 5);

      SetSizer(pFrameSizer);
      pFrameSizer->Fit(this);
      Layout();
    }

      private:

      std::tuple<Args...> mFields;
    };
  }

