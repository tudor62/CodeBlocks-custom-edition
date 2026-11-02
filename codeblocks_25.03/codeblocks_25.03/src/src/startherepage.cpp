/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU General Public License, version 3
 * http://www.gnu.org/licenses/gpl-3.0.html
 *
 * $Revision: 13293 $
 * $Id: startherepage.cpp 13293 2023-05-30 15:53:23Z mortenmacfly $
 * $HeadURL: https://svn.code.sf.net/p/codeblocks/code/branches/release-25.03/src/src/startherepage.cpp $
 */

#include <sdk.h>

#ifndef CB_PRECOMP
    #include <wx/dataobj.h>
    #include <wx/intl.h>
    #include <wx/utils.h>
    #include <wx/sizer.h>
    #include <wx/settings.h>

    #include <manager.h>
    #include <logmanager.h>
    #include <projectmanager.h>
    #include <templatemanager.h>
    #include <pluginmanager.h>
    #include <editormanager.h>
    #include <configmanager.h>
#endif

#include "startherepage.h"
#include "main.h"
#include "appglobals.h"
#include "cbcolourmanager.h"
#include "globals.h"
#include "recentitemslist.h"

#include <wx/bmpbndl.h>
#include <wx/clipbrd.h>
#include <wx/dir.h>
#include <wx/docview.h>
#include <wx/filefn.h>
#include <wx/filename.h>
#include <wx/filesys.h>
#include <wx/image.h>
#include <wx/wxhtml.h>

#include <map>

int idWin = wxNewId();

static long s_SvgCacheCounter = 0;

wxString GetStartHereTitle()
{
    return _("Start here");
}

class MyHtmlWin : public wxHtmlWindow
{
    public:
        MyHtmlWin(StartHerePage* parent, int id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHW_SCROLLBAR_AUTO)
            : wxHtmlWindow(parent, id, pos, size, style),
            m_pOwner(parent),
            m_SvgCacheDir(CreateSvgCacheDir())
        {
        }

        ~MyHtmlWin() override
        {
            for (const auto& entry : m_SvgCache)
            {
                const wxString& url = entry.second;
                if (url.StartsWith("file:///"))
                {
                    wxString path = url.Mid(8);
                    path.Replace("/", "\\");
                    if (wxFileExists(path))
                        wxRemoveFile(path);
                }
            }
            if (!m_SvgCacheDir.empty() && wxDirExists(m_SvgCacheDir))
                wxRmdir(m_SvgCacheDir);
        }

        void OnLinkClicked(const wxHtmlLinkInfo& link) override
        {
            if (m_pOwner)
            {
                if (!m_pOwner->LinkClicked(link))
                    wxLaunchDefaultBrowser(link.GetHref());
            }
        }

        wxHtmlOpeningStatus OnOpeningURL(wxHtmlURLType type, const wxString& url,
                                         wxString* redirect) const override
        {
#ifdef wxHAS_SVG
            // wxHtmlWindow can only load raster images. The Start Here page references the
            // original SVG icons, so render them through wxBitmapBundle and redirect the
            // image cell to the generated PNG. This keeps the SVG sources and gives us
            // High-DPI crispness.
            if (type == wxHTML_URL_IMAGE && url.Lower().EndsWith(".svg"))
            {
                wxString pngUrl;
                if (RenderSvgToPng(url, pngUrl))
                {
                    *redirect = pngUrl;
                    return wxHTML_REDIRECT;
                }
            }
#endif
            return wxHtmlWindow::OnOpeningURL(type, url, redirect);
        }

    private:
        static wxString CreateSvgCacheDir()
        {
            wxString dir = wxFileName::GetTempDir();
            if (dir.empty())
                return wxString();
            if (!dir.EndsWith(wxFILE_SEP_PATH))
                dir += wxFILE_SEP_PATH;
            dir += wxString::Format("cb-starthere-%lu-%ld", wxGetProcessId(), ++s_SvgCacheCounter);
            if (!wxDirExists(dir) && !wxMkdir(dir))
                return wxString();
            return dir;
        }

        bool RenderSvgToPng(const wxString& url, wxString& pngUrl) const
        {
#ifdef wxHAS_SVG
            if (m_SvgCacheDir.empty())
                return false;

            const bool dark = cbIsAppThemeDark();
            const wxString cacheKey = url + (dark ? "#dark" : "#light");
            const auto cached = m_SvgCache.find(cacheKey);
            if (cached != m_SvgCache.end())
            {
                pngUrl = cached->second;
                return true;
            }

            // The URL may be a plain file path, a file:// URL or a zip location such as
            // "start_here.zip#zip:new.svg". Strip everything up to the last separator so
            // that only the file name remains.
            wxString name = url;
            const int lastSlash = std::max(name.Find('/', true), name.Find('\\', true));
            const int lastColon = name.Find(':', true);
            const int lastSeparator = std::max(lastSlash, lastColon);
            if (lastSeparator != wxNOT_FOUND)
                name = name.Mid(lastSeparator + 1);
            if (!name.Lower().EndsWith(".svg"))
                return false;

            const wxString baseName = name.BeforeLast('.').Lower();
            int displaySize = 24;
            if (baseName == "new" || baseName == "open" || baseName == "reopen" || baseName == "tip")
                displaySize = 40;
            else if (baseName == "trash")
                displaySize = 16;

            wxFileSystem fs;
            wxFSFile* file = fs.OpenFile(url);
            if (!file)
                file = fs.OpenFile(ConfigManager::ReadDataPath() + "/start_here.zip#zip:" + name);
            if (!file)
                return false;

            wxString svg;
            wxInputStream* stream = file->GetStream();
            if (stream && stream->IsOk())
            {
                char buffer[4096];
                while (!stream->Eof() && stream->CanRead())
                {
                    stream->Read(buffer, sizeof(buffer));
                    const size_t count = stream->LastRead();
                    if (count == 0)
                        break;
                    svg += wxString::FromUTF8(buffer, count);
                }
            }
            delete file;

            if (svg.empty())
                return false;

            // The supplied SVGs use currentColor for most strokes. wxBitmapBundle renders
            // that as black by default, which would be invisible on the dark Start Here
            // background. Substitute a readable colour for the active theme.
            svg.Replace("currentColor", dark ? "#E2E8F0" : "#333333");
            const wxScopedCharBuffer utf8 = svg.ToUTF8();
            wxBitmapBundle bundle = wxBitmapBundle::FromSVG(
                reinterpret_cast<const wxByte*>(utf8.data()), utf8.length(), wxSize(24, 24));
            if (!bundle.IsOk())
                return false;

            // Render at 4x the display size, so the HTML <img> can scale down cleanly on
            // High-DPI displays.
            const int renderSize = displaySize * 4;
            wxBitmap bitmap = bundle.GetBitmap(wxSize(renderSize, renderSize));
            if (!bitmap.IsOk())
                return false;

            wxImage image = bitmap.ConvertToImage();
            if (!image.IsOk())
                return false;
            if (!image.HasAlpha())
                image.InitAlpha();

            const wxString fileName = m_SvgCacheDir + wxFILE_SEP_PATH +
                baseName + (dark ? "-dark" : "-light") + ".png";
            if (!image.SaveFile(fileName, wxBITMAP_TYPE_PNG))
                return false;

            wxString redirect = "file:///" + fileName;
            redirect.Replace("\\", "/");
            m_SvgCache[cacheKey] = redirect;
            pngUrl = redirect;
            return true;
#else
            return false;
#endif
        }

        StartHerePage* m_pOwner;
        wxString m_SvgCacheDir;
        mutable std::map<wxString, wxString> m_SvgCache;
};

BEGIN_EVENT_TABLE(StartHerePage, EditorBase)
END_EVENT_TABLE()

namespace
{

wxString wrapText(const wxString &text, const wxString &textColour)
{
    return "<font color=\"" + textColour + "\">" + text + "</font>";
}

void ReplaceRecentProjectFiles(wxString &buf, const wxFileHistory &projects, const wxFileHistory &files,
                               const wxString &linkColour, const wxString &textColour)
{
    // replace history vars
    wxString links;

    links << "<table>\n<tr><td colspan=\"2\"><b>";
    links << wrapText(_("Recent projects"), textColour);
    links << "</b></td></tr>\n";
    if (projects.GetCount())
    {
        for (size_t i = 0; i < projects.GetCount(); ++i)
        {
            links << "<tr><td width=\"50\"><img alt=\"\" width=\"20\" src=\"blank.png\" />";
            links << wxString::Format("<a href=\"CB_CMD_DELETE_HISTORY_PROJECT_%zu\"><img alt=\"\" src=\"trash.svg\" width=\"16\" height=\"16\" /></a>", i + 1);
            links << "<img alt=\"\"  width=\"10\" src=\"blank.png\" /></td><td width=\"10\">";
            links << "<a href=\"CB_CMD_OPEN_HISTORY_PROJECT_" << (i + 1) << "\">"
                  << "<font color=\"" << linkColour << "\">" << projects.GetHistoryFile(i)
                  << "</font></a>";
            links << "</td></tr>\n";
        }
    }
    else
    {
        links << "<tr><td style=\"width:2em;\"></td><td>&nbsp;&nbsp;&nbsp;&nbsp;";
        links << wrapText(_("No recent projects"), textColour);
        links << "</td></tr>\n";
    }

    links << "</table>\n<table>\n<tr><td colspan=\"2\"><b>";
    links << wrapText(_("Recent files"), textColour);
    links << "</b></td></tr>\n";
    if (files.GetCount())
    {
        for (size_t i = 0; i < files.GetCount(); ++i)
        {
            links << "<tr><td width=\"50\"><img alt=\"\" width=\"20\" src=\"blank.png\" />";
            links << wxString::Format("<a href=\"CB_CMD_DELETE_HISTORY_FILE_%zu\"><img alt=\"\" src=\"trash.svg\" width=\"16\" height=\"16\" /></a>", i + 1);
            links << "<img alt=\"\"  width=\"10\" src=\"blank.png\" /></td><td width=\"10\">";
            links << "<a href=\"CB_CMD_OPEN_HISTORY_FILE_" << (i + 1) << "\">"
                  << "<font color=\"" << linkColour << "\">" << files.GetHistoryFile(i) << "</font></a>";
            links << "</td></tr>\n";
        }
    }
    else
    {
        links << "<tr><td style=\"width:2em;\"></td><td>&nbsp;&nbsp;&nbsp;&nbsp;";
        links << wrapText(_("No recent files"), textColour);
        links << "</td></tr>\n";
    }

    links << "</table>\n";


    // update page
    buf.Replace("CB_VAR_RECENT_FILES_AND_PROJECTS", links);
    buf.Replace("CB_TXT_NEW_PROJECT",    _("Create a new project"));
    buf.Replace("CB_TXT_OPEN_PROJECT",   _("Open an existing project"));
    buf.Replace("CB_TXT_OPEN_CB_FOLDER", _("Open the Code::Blocks folder"));
    buf.Replace("CB_TXT_OPEN_MODIFICATIONS", _("Open the modifications file"));
    buf.Replace("CB_TXT_TIP_OF_THE_DAY", _("Tip of the Day"));
}

void CopyToClipboard(const wxString& text)
{
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new wxTextDataObject(text));
        wxTheClipboard->Close();
    }
}

} // anonymous namespace

StartHerePage::StartHerePage(wxEvtHandler* owner, const RecentItemsList &projects,
                             const RecentItemsList &files, wxWindow* parent)
    : EditorBase(parent, GetStartHereTitle(), true),
    m_pOwner(owner),
    m_projects(projects),
    m_files(files)
{
    RegisterColours();

    //ctor
    wxBoxSizer* bs = new wxBoxSizer(wxVERTICAL);

    // avoid gtk-critical because of sizes less than -1 (can happen with wxAuiNotebook/cbAuiNotebook)
    wxSize size = GetSize();
    size.x = std::max(size.x, -1);
    size.y = std::max(size.y, -1);

    m_pWin = new MyHtmlWin(this, idWin, wxPoint(0, 0), size);

    if (cbIsAppThemeDark())
    {
        // The main frame is shown before its first layout/paint pass. Give the page and the
        // HTML window the dark background immediately, otherwise the unpainted start page
        // flashes white for a moment during startup.
        SetBackgroundColour(cbTheme::Panel());
        m_pWin->SetBackgroundColour(cbTheme::Panel());
    }

    // set default font sizes based on system default font size

    /* NOTE (mandrav#1#): wxWidgets documentation on wxHtmlWindow::SetFonts(),
    states that the sizes array accepts values from -2 to +4.
    My tests (under linux at least) have showed that it actually
    expects real point sizes. */

    wxFont systemFont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    int sizes[7];
    for (int i = 0; i < 7; ++i)
        sizes[i] = systemFont.GetPointSize();

    m_pWin->SetFonts(wxEmptyString, wxEmptyString, sizes);

    const wxString filePath(ConfigManager::ReadDataPath());

    // Load an empty page just for setting the default image path.
    // Loading start_here.html instead is slow and generates errors in the debug log
    // because the colour macros have not been translated yet
    m_pWin->LoadPage(filePath+"/start_here.zip#zip:empty_page.html");

    // Read the file so we can perform some search and replace
    wxString buf;
    wxFileSystem* fs = new wxFileSystem;
    wxFSFile* f = fs->OpenFile(filePath+"/start_here.zip#zip:start_here.html");
    if (f)
    {
        char tmp[1024];

        wxInputStream* is = f->GetStream();
        while (!is->Eof() && is->CanRead())
        {
            memset(tmp, 0, sizeof(tmp));
            is->Read(tmp, sizeof(tmp) - 1);
            buf << cbC2U((const char*)tmp);
        }

        delete f;
    }
    else
        buf = _("<html><body><h1>Welcome to Code::Blocks!</h1><br>The default start page seems to be missing...</body></html>");

    delete fs;

#if defined(_LP64) || defined(_WIN64)
    const int bit_type = 64;
#else
    const int bit_type = 32;
#endif

    revInfo.Printf("%s (%s)   ", appglobals::AppActualVersionVerb, ConfigManager::GetSvnDate());

#if defined(__clang__)
    revInfo += wxString::Format("clang %d.%d.%d ", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__GNUC__)
    revInfo += wxString::Format("gcc %d.%d.%d ", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#endif
    revInfo += wxString::Format("%s/%s - %d bit", appglobals::AppPlatform,
                                appglobals::AppWXAnsiUnicode, bit_type);

    // The Start Here page shows a custom edition label instead of the long build string.
    // revInfo is kept intact and is still copied to the clipboard when the logo is
    // clicked, so the exact build details remain available.
    const wxString customVersionLabel =
        _T("Code::Blocks Custom Edition (Dark Optimized) &#8226; by tudor62");

    // perform var substitution
    buf.Replace("CB_VAR_REVISION_INFO", customVersionLabel);
    buf.Replace("CB_VAR_VERSION_VERB", appglobals::AppActualVersionVerb);
    buf.Replace("CB_VAR_VERSION", appglobals::AppActualVersion);
    buf.Replace("CB_SAFE_MODE", PluginManager::GetSafeMode() ? _("SAFE MODE") : wxString());

    m_OriginalPageContent = buf; // keep a copy of original for Reload()
    Reload();

    bs->Add(m_pWin, 1, wxEXPAND);
    SetSizer(bs);
    SetAutoLayout(true);
}

StartHerePage::~StartHerePage()
{
    //dtor
    //m_pWin->Destroy();
}

void StartHerePage::RegisterColours()
{
    static bool inited = false;
    if (inited)
        return;
    inited = true;

    ColourManager* cm = Manager::Get()->GetColourManager();
    cm->RegisterColour(_("Start here page"), _("Background colour"), "start_here_background",
                       wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    cm->RegisterColour(_("Start here page"), _("Link colour"), "start_here_link", *wxBLUE);
    cm->RegisterColour(_("Start here page"), _("Text colour"), "start_here_text",
                       wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
}

void StartHerePage::Reload()
{
    // Called every time something in the history changes.
    wxString buf = m_OriginalPageContent;

    ColourManager* cm = Manager::Get()->GetColourManager();
    wxColour background = cm->GetColour("start_here_background");
    wxColour link       = cm->GetColour("start_here_link");
    wxColour text       = cm->GetColour("start_here_text");

    if (cbIsAppThemeDark())
    {
        // Use read-friendly dark colours for the default (unmodified) colours and keep
        // any custom colours selected by the user.
        if (background == cm->GetDefaultColour("start_here_background"))
            background = cbTheme::Panel();
        if (text == cm->GetDefaultColour("start_here_text"))
            text = cbTheme::Text();
        if (link == cm->GetDefaultColour("start_here_link"))
            link = cbTheme::SyntaxKeyword();
    }

    const wxString backgroundColour = background.GetAsString(wxC2S_HTML_SYNTAX);
    const wxString linkColour       = link.GetAsString(wxC2S_HTML_SYNTAX);
    const wxString textColour       = text.GetAsString(wxC2S_HTML_SYNTAX);

    ReplaceRecentProjectFiles(buf, *m_projects.GetFileHistory(), *m_files.GetFileHistory(), linkColour, textColour);

    buf.Replace("CB_BODY_BGCOLOUR", backgroundColour);
    buf.Replace("CB_LINK_COLOUR",   linkColour);
    buf.Replace("CB_TEXT_COLOUR",   textColour);

    int x, y;
    m_pWin->GetViewStart(&x, &y);
    m_pWin->SetPage(buf);
    m_pWin->Scroll(x, y);
}

bool StartHerePage::LinkClicked(const wxHtmlLinkInfo& link)
{
    //If it's already loading something, stop here
    if (Manager::Get()->GetProjectManager()->IsLoading())
        return true;

    if (!m_pOwner)
        return true;

    wxString href = link.GetHref();
    if (href.StartsWith("CB_CMD_"))
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, idStartHerePageLink);
        evt.SetString(link.GetHref());
        wxPostEvent(m_pOwner, evt);
        return true;
    }

    if (   href.IsSameAs("https://www.codeblocks.org/")
        || href.StartsWith("https://sourceforge.net/p/codeblocks/tickets"))
    {
        CopyToClipboard(revInfo);
        return false;
    }

    if (href.IsSameAs("rev"))
    {
        CopyToClipboard(revInfo);
        return true;
    }

    return false;
}
