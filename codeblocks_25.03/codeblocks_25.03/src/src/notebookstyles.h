/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 */

#ifndef NOTEBOOKSTYLES_H
#define NOTEBOOKSTYLES_H

#include "cbauibook.h"

class wxDC;
class wxWindow;
class wxRect;
class wxString;
class wxBitmap;

wxColor wxAuiStepColour(const wxColor& c, int percent);

class NbStyleVC71 : public wxAuiDefaultTabArt
{
public:
    NbStyleVC71();
    wxAuiTabArt* Clone() override;

    void DrawTab(wxDC& dc, wxWindow* wnd, const wxAuiNotebookPage& page,
                        const wxRect& in_rect, int close_button_state,
                        wxRect* out_tab_rect, wxRect* out_button_rect,
                        int* x_extent) override;

    int GetBestTabCtrlSize(wxWindow* wnd, const wxAuiNotebookPageArray& pages,
                            const wxSize& required_bmp_size) override;
};

class NbStyleFF2 : public wxAuiDefaultTabArt
{
public:
    NbStyleFF2();
    wxAuiTabArt* Clone() override;
    void DrawTab(wxDC& dc, wxWindow* wnd, const wxAuiNotebookPage& page,
                        const wxRect& in_rect, int close_button_state,
                        wxRect* out_tab_rect, wxRect* out_button_rect,
                        int* x_extent) override;

    int GetBestTabCtrlSize(wxWindow* wnd, const wxAuiNotebookPageArray& pages,
                            const wxSize& required_bmp_size) override;
};

/** Modern, thin dark tab art used by all AUI notebooks in the Full Dark Theme.
    The active tab uses the editor colour, inactive tabs blend into the window colour.
    It derives from the generic (not the native MSW) tab art so that all drawing is
    under our control and nothing falls back to a white system colour. */
class NbStyleDark : public wxAuiGenericTabArt
{
public:
    NbStyleDark();
    wxAuiTabArt* Clone() override;

    void DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;
    void DrawTab(wxDC& dc, wxWindow* wnd, const wxAuiNotebookPage& page,
                 const wxRect& in_rect, int close_button_state,
                 wxRect* out_tab_rect, wxRect* out_button_rect,
                 int* x_extent) override;
    void DrawButton(wxDC& dc, wxWindow* wnd, const wxRect& in_rect,
                    int bitmapId, int buttonState, int orientation,
                    wxRect* outRect) override;
    void DrawBorder(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;

    int GetBorderWidth(wxWindow* wnd) override;
    int GetAdditionalBorderSpace(wxWindow* wnd) override;
};

#endif // NOTEBOOKSTYLES_H
