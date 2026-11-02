/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision: 12643 $
 * $Id: loggers.cpp 12643 2022-01-12 19:40:38Z wh11204 $
 * $HeadURL: https://svn.code.sf.net/p/codeblocks/code/branches/release-25.03/src/sdk/loggers.cpp $
 */

#include "sdk_precomp.h"
#ifndef CB_PRECOMP
    #include <wx/listctrl.h>
    #include <wx/textctrl.h>
    #include <wx/settings.h>
    #include "cbexception.h"
    #include "manager.h"
    #include "configmanager.h"
#endif

#include <wx/clipbrd.h>
#include <wx/dataobj.h>
#include <wx/wupdlock.h>
#include <wx/app.h>
#include <wx/thread.h>
#include <wx/timer.h>

#include <map>
#include <mutex>
#include <vector>

#include "loggers.h"
#include "cbcolourmanager.h"
#include "globals.h"

namespace
{
const int kLoggerBatchFlushMs = 50;

struct TextBatchEntry
{
    wxString message;
    Logger::level level;
};

struct ListBatchEntry
{
    wxArrayString columns;
    Logger::level level;
    int autoSize;
};

std::mutex g_loggerBatchMutex;
std::map<TextCtrlLogger*, std::vector<TextBatchEntry>> g_textBatches;
std::map<ListCtrlLogger*, std::vector<ListBatchEntry>> g_listBatches;

/** Flushes the queued log messages from a single short-lived timer on the GUI thread. */
class LoggerBatchFlusher : public wxEvtHandler
{
public:
    LoggerBatchFlusher() : m_timer(this)
    {
        Bind(wxEVT_TIMER, &LoggerBatchFlusher::OnTimer, this);
    }

    void Schedule()
    {
        if (!m_timer.IsRunning())
            m_timer.StartOnce(kLoggerBatchFlushMs);
    }

private:
    void OnTimer(wxTimerEvent&)
    {
        std::vector<TextCtrlLogger*> textLoggers;
        std::vector<ListCtrlLogger*> listLoggers;

        {
            std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
            for (const auto& item : g_textBatches)
            {
                if (!item.second.empty())
                    textLoggers.push_back(item.first);
            }
            for (const auto& item : g_listBatches)
            {
                if (!item.second.empty())
                    listLoggers.push_back(item.first);
            }
        }

        for (TextCtrlLogger* logger : textLoggers)
            logger->FlushPending();
        for (ListCtrlLogger* logger : listLoggers)
            logger->FlushPending();

        // Messages can arrive while the batch is being written (for example from an event
        // handler triggered by the control). Make sure they get their own flush.
        bool pending = false;
        {
            std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
            for (const auto& item : g_textBatches)
            {
                if (!item.second.empty())
                {
                    pending = true;
                    break;
                }
            }
            if (!pending)
            {
                for (const auto& item : g_listBatches)
                {
                    if (!item.second.empty())
                    {
                        pending = true;
                        break;
                    }
                }
            }
        }
        if (pending)
            Schedule();
    }

    wxTimer m_timer;
};

LoggerBatchFlusher* g_loggerBatchFlusher = nullptr;

void RequestBatchFlush()
{
    if (wxThread::IsMain())
    {
        if (!g_loggerBatchFlusher)
            g_loggerBatchFlusher = new LoggerBatchFlusher;
        g_loggerBatchFlusher->Schedule();
    }
    else if (wxTheApp)
    {
        // Timers and controls must only be touched from the GUI thread.
        wxTheApp->CallAfter([]() { RequestBatchFlush(); });
    }
}

void QueueTextMessage(TextCtrlLogger* logger, const wxString& message, Logger::level level)
{
    {
        std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
        g_textBatches[logger].push_back(TextBatchEntry{message, level});
    }
    RequestBatchFlush();
}

void QueueListMessage(ListCtrlLogger* logger, const wxArrayString& columns,
                      Logger::level level, int autoSize)
{
    {
        std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
        g_listBatches[logger].push_back(ListBatchEntry{columns, level, autoSize});
    }
    RequestBatchFlush();
}

void DiscardTextMessages(TextCtrlLogger* logger)
{
    std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
    g_textBatches.erase(logger);
}

void DiscardListMessages(ListCtrlLogger* logger)
{
    std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
    g_listBatches.erase(logger);
}
} // anonymous namespace

// Helper function which blends a colour with the default window text colour,
// so that text will be readable in bright and dark colour schemes
wxColour BlendTextColour(wxColour col)
{
    wxColour fgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    wxColour bgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);

    int dist = (fgCol.Red()*fgCol.Red() + fgCol.Green()*fgCol.Green() + fgCol.Blue()*fgCol.Blue())
             - (bgCol.Red()*bgCol.Red() + bgCol.Green()*bgCol.Green() + bgCol.Blue()*bgCol.Blue());
    if (dist > 0)
    {
        // If foreground color is brighter than background color, this is a dark theme, so
        // brighten the text colour.
        // I would use wxColour::changeLightness(), but it's only available in v2.9.0 or later.
        int d = int(sqrt(dist)/4);
        int r = col.Red()   + d;
        int g = col.Green() + d;
        int b = col.Blue()  + d;
        return wxColour( r>255 ? 255 : r, g>255 ? 255 : g, b>255 ? 255 : b );
    }
    return col;
}

TextCtrlLogger::TextCtrlLogger(bool fixedPitchFont) :
    control(nullptr), fixed(fixedPitchFont)
{
}

TextCtrlLogger::~TextCtrlLogger()
{
    DiscardTextMessages(this);
    control = nullptr; // invalidate, do NOT destroy
}

void TextCtrlLogger::CopyContentsToClipboard(bool selectionOnly)
{
    // Make sure a just-finished build is fully visible before copying it.
    FlushPending();

    if (control && control->GetValue().IsEmpty() == false && wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(selectionOnly ? control->GetStringSelection() : control->GetValue()));
        wxTheClipboard->Close();
    }
}

void TextCtrlLogger::UpdateSettings()
{
    if (!control)
        return;

    const bool dark = cbIsAppThemeDark();
    const wxColour background = dark ? cbTheme::Log()
                                     : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const wxColour foreground = dark ? cbTheme::Text()
                                     : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    control->SetBackgroundColour(background);
    control->SetForegroundColour(foreground);

    ConfigManager* cfgman = Manager::Get()->GetConfigManager(_T("message_manager"));
    int size = cfgman->ReadInt(_T("/log_font_size"), (platform::macosx ? 10 : 8));

    wxFont default_font(size, fixed ? wxFONTFAMILY_MODERN : wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont bold_font(default_font);
    wxFont italic_font(default_font);

    bold_font.SetWeight(wxFONTWEIGHT_BOLD);

    wxFont bigger_font(bold_font);
    bigger_font.SetPointSize(size + 2);

    wxFont small_font(default_font);
    small_font.SetPointSize(size - 4);

    italic_font.SetStyle(wxFONTSTYLE_ITALIC);

    // might try alternatively
    //italic_font.SetStyle(wxFONTSTYLE_SLANT);

    for(unsigned int i = 0; i < num_levels; ++i)
    {
        style[i].SetFont(default_font);
        style[i].SetAlignment(wxTEXT_ALIGNMENT_DEFAULT);
        style[i].SetTextColour(foreground);
        style[i].SetBackgroundColour(background);

        // is it necessary to do that?
        //style[i].SetFlags(...);
    }

    style[caption].SetAlignment(wxTEXT_ALIGNMENT_CENTRE);
    bigger_font.SetUnderlined(true);
    style[caption].SetFont(bigger_font);

    ColourManager *colours = Manager::Get()->GetColourManager();

    style[success].SetTextColour(colours->GetColour(wxT("logs_success_text")));

    style[warning].SetFont(italic_font);
    style[warning].SetTextColour(colours->GetColour(wxT("logs_warning_text")));

    style[error].SetFont(bold_font);
    style[error].SetTextColour(colours->GetColour(wxT("logs_error_text")));

    style[critical].SetFont(bold_font);
    style[critical].SetTextColour(colours->GetColour(wxT("logs_critical_text")));     // we're setting both fore and background colors here
    style[critical].SetBackgroundColour(colours->GetColour(wxT("logs_critical_back"))); // so we don't have to mix in default colors
    style[spacer].SetFont(small_font);

    if (dark)
    {
        // Keep GCC build output clearly readable on the very dark log background. Normal
        // output stays white, warnings use amber, errors use a soft red and successful
        // messages use the subtle green from the syntax palette.
        style[caption].SetTextColour(cbTheme::Text());
        style[success].SetTextColour(cbTheme::SyntaxComment());
        style[failure].SetTextColour(wxColour(0xfb, 0x71, 0x85));
        style[warning].SetTextColour(wxColour(0xfb, 0xbf, 0x24));
        style[error].SetTextColour(wxColour(0xf8, 0x71, 0x71));
        style[critical].SetTextColour(wxColour(0xfe, 0xca, 0xca));
        style[critical].SetBackgroundColour(wxColour(0x7f, 0x1d, 0x1d));
    }

    // Tell control about the font change
    control->SetFont(default_font);

    // Pending messages must use the new styles and font too.
    FlushPending();

    control->Refresh();
} // end of UpdateSettings

void TextCtrlLogger::Append(const wxString& msg, Logger::level lv)
{
    if (!control)
    {
        // Do the same as the stdout logger, to prevent loss of messages during start up!
        fputs(wxSafeConvertWX2MB(msg.wc_str()), lv < error ? stdout : stderr);
        fputs(::newline_string.mb_str(), lv < error ? stdout : stderr);
        return;
    }

    // Queue the message and let the shared 50 ms timer write a whole burst in one go.
    QueueTextMessage(this, msg, lv);
}

void TextCtrlLogger::FlushPending()
{
    std::vector<TextBatchEntry> batch;
    {
        std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
        auto it = g_textBatches.find(this);
        if (it == g_textBatches.end())
            return;
        batch.swap(it->second);
        g_textBatches.erase(it);
    }

    if (!control || batch.empty())
        return;

    // One freeze/thaw and one final scroll position for the complete batch.
    control->Freeze();
    for (const TextBatchEntry& entry : batch)
    {
        wxString text(entry.message);
        text.Append(_T('\n'));

        if (entry.level == caption)
        {
            control->SetDefaultStyle(style[info]);
            control->AppendText(::newline_string);

            control->SetDefaultStyle(style[entry.level]);
            control->AppendText(text);

            control->SetDefaultStyle(style[spacer]);
            control->AppendText(::newline_string);
        }
        else
        {
            control->SetDefaultStyle(style[entry.level]);
            control->AppendText(text);
        }
    }
    control->ShowPosition(control->GetLastPosition());
    control->Thaw();
}

void TextCtrlLogger::Clear()
{
    // Messages which have not been flushed yet belong to the log that is being cleared.
    DiscardTextMessages(this);

    if (control)
        control->Clear();
}

wxWindow* TextCtrlLogger::CreateControl(wxWindow* parent)
{
    if (!control)
        control = new wxTextCtrl(parent, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH | wxTE_NOHIDESEL | wxTE_AUTO_URL);
    return control;
}


bool TextCtrlLogger::HasFeature(Feature::Enum feature) const
{
    switch (feature)
    {
        case Feature::IsWrappable:
            return (control && control->IsMultiLine());
        case Feature::CanClear:
        case Feature::CanCopy:
            return true;
        default:
            return false;
    }
}

bool TextCtrlLogger::GetWrapMode() const
{
    if (!control)
        return false;

    long ctrl_style = control->GetWindowStyle();
    return ((ctrl_style & wxTE_DONTWRAP)!=wxTE_DONTWRAP);
}

void TextCtrlLogger::ToggleWrapMode()
{
    if (!control || !HasFeature(Feature::IsWrappable))
        return;

    long ctrl_style = control->GetWindowStyle();

    // wxTE_DONTWRAP is an equivalent for wxHSCROLL (see <wx/textctrl.h>)
    bool is_dontwrap = ((ctrl_style & wxTE_DONTWRAP)==wxTE_DONTWRAP);
    if (is_dontwrap)
        ctrl_style &= ~wxTE_DONTWRAP; // don't wrap = OFF means wrapping = ON
    else
        ctrl_style |=  wxTE_DONTWRAP; // don't wrap = ON means wrapping = OFF

// On Windows the wrap-style can not easily be changed on the fly, but if the align flags
// change wxWidgets recreates the textcontrol, doing this ourselves seems not to be so easy,
// so we use this hack here (testing if the centred-flag is set, is not needed for our loggers)
#ifdef __WXMSW__
    bool is_centred = ((ctrl_style & wxALIGN_CENTER)==wxALIGN_CENTER);
    if (is_centred)
    {
        ctrl_style &= ~wxALIGN_CENTRE;
        control->SetWindowStyleFlag(ctrl_style);
        ctrl_style |= wxALIGN_CENTRE;
    }
    else
    {
        ctrl_style |= wxALIGN_CENTRE;
        control->SetWindowStyleFlag(ctrl_style);
        ctrl_style &= ~wxALIGN_CENTRE;
    }
#endif
    control->SetWindowStyleFlag(ctrl_style);
}


void TimestampTextCtrlLogger::Append(const wxString& msg, Logger::level lv)
{
    if (!control)
        return;

    wxDateTime timestamp(wxDateTime::UNow());
    wxString text;
    text.Printf(_T("[%2.2d:%2.2d:%2.2d.%3.3d] %s"), timestamp.GetHour(), timestamp.GetMinute(), timestamp.GetSecond(), timestamp.GetMillisecond(), msg.c_str());

    // The timestamp is generated now, not when the batch is flushed.
    QueueTextMessage(this, text, lv);
}

ListCtrlLogger::ListCtrlLogger(const wxArrayString& titles_in, const wxArrayInt& widths_in, bool fixedPitchFont) :
    control(nullptr),
    fixed(fixedPitchFont),
    titles(titles_in),
    widths(widths_in)
{
    cbAssert(titles.GetCount() == widths.GetCount());
}

ListCtrlLogger::~ListCtrlLogger()
{
    DiscardListMessages(this);
    control = nullptr; // invalidate, do NOT destroy
}

void ListCtrlLogger::CopyContentsToClipboard(bool selectionOnly)
{
    // Make sure a just-finished build is fully visible before copying it.
    FlushPending();

    if (control && control->GetItemCount() > 0 && wxTheClipboard->Open())
    {
        wxString text;
        if (selectionOnly)
        {
            int i = control->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            text = GetItemAsText(i);
        }
        else
        {
            for (int i = 0; i < control->GetItemCount(); ++i)
            {
                text << GetItemAsText(i);
                if (platform::windows)
                {
                    text << _T('\r'); // Add CR for Windows clipboard
                }
                text << _T('\n');
            }
        }
        wxTheClipboard->SetData(new wxTextDataObject(text));
        wxTheClipboard->Close();
    }
}

wxString ListCtrlLogger::GetItemAsText(long item) const
{
    if (!control)
        return wxEmptyString;

    wxString text;

    wxListItem li_info;
    li_info.SetId(item);
    li_info.SetMask(wxLIST_MASK_TEXT);

    for (size_t i = 0; i < titles.GetCount(); ++i)
    {
        li_info.SetColumn(i);
        control->GetItem(li_info);
        text << li_info.GetText() << _T('|');
    }
    return text;
} // end of GetItemAsText

void ListCtrlLogger::UpdateSettings()
{
    if (!control)
        return;

    ConfigManager* cfgman = Manager::Get()->GetConfigManager(_T("message_manager"));
    int size = cfgman->ReadInt(_T("/log_font_size"), (platform::macosx ? 10 : 8));
    wxFont default_font(size, fixed ? wxFONTFAMILY_MODERN : wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    wxFont bold_font(default_font);
    wxFont italic_font(default_font);

    bold_font.SetWeight(wxFONTWEIGHT_BOLD);

    wxFont bigger_font(bold_font);
    bigger_font.SetPointSize(size + 2);

    wxFont small_font(default_font);
    small_font.SetPointSize(size - 4);

    italic_font.SetStyle(wxFONTSTYLE_ITALIC);

    const bool dark = cbIsAppThemeDark();
    const wxColour background = dark ? cbTheme::Log()
                                     : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const wxColour foreground = dark ? cbTheme::Text()
                                     : wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    for(unsigned int i = 0; i < num_levels; ++i)
    {
        style[i].font = default_font;
        style[i].colour = foreground;
    }

    ColourManager *colours = Manager::Get()->GetColourManager();

    style[caption].font = bigger_font;
    style[success].colour = colours->GetColour(wxT("logs_success_text"));
    style[failure].colour = colours->GetColour(wxT("logs_failure_text"));

    style[warning].font = italic_font;
    style[warning].colour = colours->GetColour(wxT("logs_warning_text"));

    style[error].colour = colours->GetColour(wxT("logs_error_text"));

    style[critical].font = bold_font;
    style[critical].colour = colours->GetColour(wxT("logs_critical_text_listctrl"));

    style[spacer].font = small_font;
    style[pagetitle] = style[caption];

    if (dark)
    {
        style[caption].colour = cbTheme::Text();
        style[success].colour = cbTheme::SyntaxComment();
        style[failure].colour = wxColour(0xfb, 0x71, 0x85);
        style[warning].colour = wxColour(0xfb, 0xbf, 0x24);
        style[error].colour = wxColour(0xf8, 0x71, 0x71);
        style[critical].colour = wxColour(0xfe, 0xca, 0xca);
    }

    // Insert queued rows with the new styles before updating the existing items.
    FlushPending();

    control->SetBackgroundColour(background);
    control->SetForegroundColour(foreground);

    // Tell control and items about the font change
    control->SetFont(default_font);
    for (int i = 0; i < control->GetItemCount(); ++i)
    {
        wxFont font = control->GetItemFont(i);
        font.SetPointSize(size);
        control->SetItemFont( i, font );
    }//for
} // end of UpdateSettings

void ListCtrlLogger::Append(const wxString& msg, Logger::level lv)
{
    if (!control)
        return;

    wxArrayString columns;
    columns.Add(msg);
    QueueListMessage(this, columns, lv, -1);
}

void ListCtrlLogger::Append(const wxArrayString& colValues, Logger::level lv, int autoSize)
{
    if (!control)
        return;

    if (colValues.GetCount() == 0 || colValues.GetCount() > titles.GetCount())
        return;

    QueueListMessage(this, colValues, lv, autoSize);
}

void ListCtrlLogger::FlushPending()
{
    std::vector<ListBatchEntry> batch;
    {
        std::lock_guard<std::mutex> lock(g_loggerBatchMutex);
        auto it = g_listBatches.find(this);
        if (it == g_listBatches.end())
            return;
        batch.swap(it->second);
        g_listBatches.erase(it);
    }

    if (!control || batch.empty())
        return;

    int idx = control->GetItemCount();

    bool autoScroll = false;
#if wxCHECK_VERSION(3, 1, 3)
    autoScroll = (idx > 0) && control->IsVisible(idx - 1);
#endif

    int autoSize = -1;

    // One freeze/thaw, one auto-scroll and at most one column auto-fit for the whole batch.
    control->Freeze();
    for (const ListBatchEntry& entry : batch)
    {
        if (entry.columns.IsEmpty())
            continue;

        control->InsertItem(idx, entry.columns[0]);
        control->SetItemFont(idx, style[entry.level].font);
        control->SetItemTextColour(idx, style[entry.level].colour);

        for (size_t i = 1; i < entry.columns.GetCount() && i < titles.GetCount(); ++i)
            control->SetItem(idx, i, entry.columns[i]);

        if (entry.autoSize >= 0)
            autoSize = entry.autoSize;

        ++idx;
    }

    // Call once per batch; the base implementation ignores -1 and derived classes such as
    // CompilerMessages use it to auto-fit all columns during a build.
    AutoFitColumns(autoSize);

    if (autoScroll && idx > 0)
        control->EnsureVisible(idx - 1);

    control->Thaw();
}

size_t ListCtrlLogger::GetItemsCount() const
{
    // Callers use the item count as an index into the error list; make sure queued rows
    // are visible so the count and the control cannot get out of sync.
    const_cast<ListCtrlLogger*>(this)->FlushPending();

    return control ? control->GetItemCount() : 0;
}

void ListCtrlLogger::Clear()
{
    // Rows which have not been flushed yet belong to the list that is being cleared.
    DiscardListMessages(this);

    if (control)
        control->DeleteAllItems();
}

wxWindow* ListCtrlLogger::CreateControl(wxWindow* parent)
{
    if (control)
        return control;

    control = new wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
    for (size_t i = 0; i < titles.GetCount(); ++i)
        control->InsertColumn(i, titles[i], wxLIST_FORMAT_LEFT, widths[i]);

    return control;
}

bool ListCtrlLogger::HasFeature(Feature::Enum feature) const
{
    switch (feature)
    {
    case Feature::CanClear:
        return true;
    case Feature::CanCopy:
        return true;
    default:
        return false;
    }
}

void ListCtrlLogger::AutoFitColumns(int column)
{
    if (column != -1)
        control->SetColumnWidth(column, wxLIST_AUTOSIZE);
}

CSS::CSS() :
    caption  (_T("font-size: 12pt;")),
    info     (wxEmptyString),
    warning  (_T("margin-left: 2em;")),
    success  (wxEmptyString),
    error    (_T("margin-left: 2em; border-left: 1px solid ")),
    critical (_T("color: ")),
    failure  (_T("color: ")),
    pagetitle(_T("font-size: 16pt;")),
    spacer   (wxEmptyString),
    asterisk (_T("font-family: Arial, Helvetica, \"Bitstream Vera Sans\", sans;"))
{
    error    += BlendTextColour(*wxRED).GetAsString(wxC2S_HTML_SYNTAX) + _T(";");
    critical += BlendTextColour(*wxRED).GetAsString(wxC2S_HTML_SYNTAX) + _T("; font-weight: bold;");
    failure  += BlendTextColour(wxColour(0x80, 0x00, 0x00)).GetAsString(wxC2S_HTML_SYNTAX) + _T(";");
}

CSS::operator wxString()
{
    wxString tmp(_T('\0'), 300);
    return tmp;
}

HTMLFileLogger::HTMLFileLogger(const wxString& filename)
{
    Open(filename);
}



void HTMLFileLogger::Append(const wxString& msg, cb_unused Logger::level lv)
{
    fputs(wxSafeConvertWX2MB(msg.wc_str()), f.fp());
    fputs(::newline_string.mb_str(), f.fp());
}

typedef const char* wxStringSucks;


void HTMLFileLogger::Open(const wxString& filename)
{
    FileLogger::Open(filename);
    fputs("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en-US\" lang=\"en-US\">\n<head>\n<title>Build log</title>\n<style>\n", f.fp());

    if (!!css.asterisk)
        fprintf(f.fp(), "* { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.asterisk.wc_str()));
    if (!!css.pagetitle)
        fprintf(f.fp(), "h1 { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.pagetitle.wc_str()));
    if (!!css.caption)
        fprintf(f.fp(), "h2 { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.caption.wc_str()));
    if (!!css.info)
        fprintf(f.fp(), ".info { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.info.wc_str()));
    if (!!css.warning)
        fprintf(f.fp(), ".warn { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.warning.wc_str()));
    if (!!css.error)
        fprintf(f.fp(), ".error { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.error.wc_str()));
    if (!!css.success)
        fprintf(f.fp(), ".success { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.success.wc_str()));
    if (!!css.failure)
        fprintf(f.fp(), ".failure { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.failure.wc_str()));
    if (!!css.critical)
        fprintf(f.fp(), ".critical { %s }\n", (wxStringSucks) wxSafeConvertWX2MB(css.critical.wc_str()));
    fputs("</style>\n</head>\n\n<body>", f.fp());
}

void HTMLFileLogger::Close()
{
    fputs("</body>\n</html>\n", f.fp());
    FileLogger::Close();
}
