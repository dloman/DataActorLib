#pragma once

#include <TypeTraits/TypeTraits.hpp>
#include <GuiStuff/Helpers.hpp>
#include <wx/dataview.h>
#include <wx/grid.h>
#include <wx/frame.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>

#include <boost/hana.hpp>
#include <boost/type_index.hpp>
#include <type_traits>
#include <iostream>
#include <locale>

class wxWindow;

namespace gs
{
  namespace
  {
    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void RemoveNamespace(std::string& name)
    {
      auto iName = name.rfind("::");

      if (iName != std::string::npos)
      {
        name = name.substr(iName + 2);
      }

      ConvertCamelCaseToSpaces(name);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template <typename PacketType>
    void AddGridLabels(wxGrid* pGrid, PacketType packet)
    {
      namespace hana = boost::hana;
      std::vector<std::string> labels;
      hana::for_each(packet, [&labels](auto pair)
      {
        labels.emplace_back(hana::to<const char*>(hana::first(pair)));
      });

      pGrid->CreateGrid(1, labels.size());
      for (auto i = 0u; i < labels.size(); ++i)
      {
        //remove 'm' prefix
        if (labels[i][0] == 'm')
        {
          labels[i] = labels[i].substr(1);
        }

        ConvertCamelCaseToSpaces(labels[i]);
        pGrid->SetColLabelValue(i, labels[i]);
        pGrid->AutoSizeColLabelSize(i);
      }
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template <typename PacketType>
    void AddGridValues(wxGrid* pGrid, PacketType packet)
    {
      namespace hana = boost::hana;
      std::vector<std::string> labels;
      hana::for_each(packet, [&labels, pGrid, i = 0] (auto pair) mutable
      {
        pGrid->SetCellValue(0, i++, std::to_string(hana::second(pair)));
      });

      pGrid->AutoSizeColumns();
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template<std::size_t Index = 0, typename GridArrayType, typename TupleType>
    typename std::enable_if_t<Index == std::tuple_size_v<TupleType>>
      AddPages(
        wxNotebook* pNotebook,
        GridArrayType& gridArray,
        const TupleType& tuple)
    {
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template<std::size_t Index = 0, typename GridArrayType, typename TupleType>
    typename std::enable_if_t<Index != std::tuple_size_v<TupleType>>
      AddPages(
        wxNotebook* pNotebook,
        GridArrayType& gridArray,
        const TupleType& tuple)
    {
      using PacketType = decltype(std::get<Index>(tuple));

      auto name = boost::typeindex::type_id<PacketType>().pretty_name();

      RemoveNamespace(name);

      auto pPage = new wxPanel(pNotebook, wxID_ANY);

      auto pGrid = new wxGrid(pPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

      gridArray[Index] = pGrid;

      // Grid
      AddGridLabels(pGrid, std::get<Index>(tuple));

      pGrid->EnableEditing(false);
      pGrid->EnableGridLines(true);
      pGrid->EnableDragGridSize();
      pGrid->SetMargins(0, 0);

      // Columns
      pGrid->EnableDragColMove(false);
      pGrid->EnableDragColSize(false);
      pGrid->AutoSizeColumns();

      // Rows
      pGrid->EnableDragRowSize(false);
      pGrid->HideRowLabels();
      pGrid->AutoSizeRows();

      // Cell Defaults
      pGrid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);

      auto pPageSizer = new wxBoxSizer(wxHORIZONTAL);

      auto Flags = wxSizerFlags(1).Center().Border(wxALL, 5);

      pPageSizer->Add(pGrid, Flags);

      pPage->SetSizer(pPageSizer);
      pPage->Layout();
      pPageSizer->Fit(pPage);

      pNotebook->AddPage(pPage, name, false);

      AddPages<Index + 1>(pNotebook, gridArray, tuple);
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template<std::size_t Index = 0, typename T, typename TupleType>
    typename std::enable_if_t<Index == std::tuple_size_v<TupleType>, size_t>
      GetIndex(const TupleType& tuple)
    {
      throw std::logic_error("unable to get index of wxGrid");
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    template<std::size_t Index = 0, typename T, typename TupleType>
    typename std::enable_if_t<Index != std::tuple_size_v<TupleType>, size_t>
      GetIndex(const TupleType& tuple)
    {
      if (
        std::is_same_v<
          std::decay_t<decltype(std::get<Index>(tuple))>,
          std::decay_t<T>>)
      {
        return Index;
      }
      return GetIndex<Index + 1, T, TupleType>(tuple);
    }
  }

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  template <typename ... Args>
  class GridDisplayer : public wxPanel
  {
    public:

      //------------------------------------------------------------------------
      //------------------------------------------------------------------------
      GridDisplayer(wxWindow* pParent)
        : wxPanel(pParent, wxID_ANY),
        mFields()
      {
        SetSizeHints(wxDefaultSize, wxDefaultSize);

        auto pPanel = new wxPanel(
          this,
          wxID_ANY,
          wxDefaultPosition,
          wxDefaultSize,
          wxTAB_TRAVERSAL);

        auto pMainSizer = new wxBoxSizer(wxHORIZONTAL);

        auto pNotebook =
          new wxNotebook(pPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

        AddPages(pNotebook, mGrids, mFields);

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

      //------------------------------------------------------------------------
      //------------------------------------------------------------------------
      template <typename T>
      void Set(T t)
      {
        static_assert(
          dl::ContainsType<T, std::tuple<Args...>> {},
          "Set must be called with contained type");

        gs::DoOnGuiThread([t, this] { AddGridValues(GetGrid<T>(), t); });
      }

    private:

      //------------------------------------------------------------------------
      //------------------------------------------------------------------------
      template <typename T>
      wxGrid* GetGrid()
      {
        return mGrids[GetIndex<0, T, std::tuple<Args...>>(mFields)];
      }

    private:

      std::tuple<Args...> mFields;

      std::array<wxGrid*, std::tuple_size_v<std::tuple<Args...>>> mGrids;
    };
  }

