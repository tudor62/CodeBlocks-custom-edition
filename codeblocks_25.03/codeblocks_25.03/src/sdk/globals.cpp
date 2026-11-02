/*
 * This file is part of the Code::Blocks IDE and licensed under the GNU Lesser General Public License, version 3
 * http://www.gnu.org/licenses/lgpl-3.0.html
 *
 * $Revision: 13330 $
 * $Id: globals.cpp 13330 2023-08-06 07:13:08Z mortenmacfly $
 * $HeadURL: https://svn.code.sf.net/p/codeblocks/code/branches/release-25.03/src/sdk/globals.cpp $
 */

#include "sdk_precomp.h"

#ifndef CB_PRECOMP
    #include <wx/choicdlg.h>
    #include <wx/dcmemory.h>
    #include <wx/file.h>
    #include <wx/filename.h>
    #include <wx/filesys.h>
    #include <wx/image.h>
    #include <wx/imaglist.h>
    #include <wx/listctrl.h>
    #include <wx/menu.h>
    #include <wx/settings.h>
    #include <wx/textdlg.h>

    #include "wx/wxscintilla.h"

    #include "cbexception.h"
    #include "configmanager.h" // ReadBool
    #include "filemanager.h"
    #include "globals.h"
    #include "logmanager.h"
    #include "manager.h"
    #include "projectmanager.h"
#endif

#include <tinyxml.h>

#include <wx/dirdlg.h>
#include <wx/display.h>
#include <wx/filefn.h>
#include <wx/fontmap.h>
#include <wx/generic/msgdlgg.h>
#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/tokenzr.h>

#include <wx/aui/auibook.h>
#include <wx/aui/dockart.h>
#include <wx/aui/framemanager.h>

#include "cbauibook.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <string>

#include "filefilters.h"
#include "tinywxuni.h"
#include "filegroupsandmasks.h"

#ifndef __WXMSW__
    #include <unistd.h> // readlink
    #include <sys/stat.h> // lstat
#else
    #include <commctrl.h>
#endif

const wxString DEFAULT_WORKSPACE     = _T("default.workspace");
const wxString DEFAULT_ARRAY_SEP     = _T(";");

#ifdef __WXMAC__
const wxString DEFAULT_CONSOLE_TERM  = _T("osascript -e 'tell app \"Terminal\"' -e 'activate' -e 'do script \"$SCRIPT\"' -e 'end tell'");
const wxString DEFAULT_CONSOLE_SHELL = _T("/bin/zsh -c");
#else
const wxString DEFAULT_CONSOLE_TERM  = _T("xterm -T $TITLE -e");
const wxString DEFAULT_CONSOLE_SHELL = _T("/bin/sh -c");
#endif

#if defined __WXMSW__
const wxString cbDEFAULT_OPEN_FOLDER_CMD = _T("explorer.exe /select,");
#elif defined __WXMAC__
const wxString cbDEFAULT_OPEN_FOLDER_CMD = _T("open -R");
#else
const wxString cbDEFAULT_OPEN_FOLDER_CMD = _T("xdg-open");
#endif

int GetPlatformsFromString(const wxString& platforms)
{
    bool pW = platforms.Contains("Windows");
    bool pU = platforms.Contains("Unix");
    bool pM = platforms.Contains("Mac");
    bool pA = platforms.Contains("All");

    if (pA || (pW && pU && pM))
        return spAll;

    int     p  = 0;
    if (pW) p |= spWindows;
    if (pU) p |= spUnix;
    if (pM) p |= spMac;
    return p;
}

wxString GetStringFromPlatforms(int platforms, bool forceSeparate)
{
    wxString ret;

    if (!forceSeparate)
    {
        int tmpAll = spWindows | spUnix | spMac;
        if (((platforms & tmpAll) == tmpAll) || ((platforms & spAll) == spAll))
            return "All";
    }

    if (platforms & spWindows)
        ret << "Windows;";
    if (platforms & spUnix)
        ret << "Unix;";
    if (platforms & spMac)
        ret << "Mac;";
    return ret;
}

/*
    Killerbot : 6 Oktober 2007
    The method has been extended with a bool to specify if the seperator should be appended at the end.
    Ultimate goal : every client gives as a value : false.
    Why : well a seperator should separate, there's nothing to separate at the end (like nothing to separate
    at the beginning, we don't put one there ...), but some client code is having problems when the separator is
    not present at the end. So for compatibility issues we have as default value for the new argument true, so the
    old code has the same behaviour as before [TODO: make those clients no longer dependent on this stupid final separator behaviour]
    New code will specify false as the bool value, so hoping rather soon a search on this method will have all hits showing
    false as the last argument ....
*/

wxString GetStringFromArray(const wxArrayString& array, const wxString& separator, bool SeparatorAtEnd)
{
    wxString out;
    for (unsigned int i = 0; i < array.GetCount(); ++i)
    {
        out << array[i];
        if (i < array.GetCount() - 1 || SeparatorAtEnd)
            out << separator;
    }
    return out;
}

wxArrayString GetArrayFromString(const wxString& text, const wxString& separator, bool trimSpaces)
{
    wxArrayString out;
    wxString search = text;
    int seplen = separator.Length();
    while (true)
    {
        int idx = search.Find(separator);
        if (idx == -1)
        {
            if (trimSpaces)
            {
                search.Trim(false);
                search.Trim(true);
            }
            if (!search.IsEmpty())
                out.Add(search);
            break;
        }
        wxString part = search.Left(idx);
        search.Remove(0, idx + seplen);
        if (trimSpaces)
        {
            part.Trim(false);
            part.Trim(true);
        }
        if (!part.IsEmpty())
            out.Add(part);
    }
    return out;
}

wxStringVec GetVectorFromString(const wxString& text, const wxString& separator, bool trimSpaces)
{
    wxStringVec out;
    wxString search = text;
    int seplen = separator.Length();
    while (true)
    {
        int idx = search.Find(separator);
        if (idx == -1)
        {
            if (trimSpaces)
            {
                search.Trim(false);
                search.Trim(true);
            }
            if (!search.IsEmpty())
                out.push_back(search);
            break;
        }
        wxString part = search.Left(idx);
        search.Remove(0, idx + seplen);
        if (trimSpaces)
        {
            part.Trim(false);
            part.Trim(true);
        }
        if (!part.IsEmpty())
            out.push_back(part);
    }
    return out;
}

wxArrayString MakeUniqueArray(const wxArrayString& array, bool caseSens)
{
    wxArrayString out;
    for (size_t i = 0; i < array.GetCount(); ++i)
    {
        if (caseSens)
        {
          if (out.Index(array[i]) == wxNOT_FOUND)
              out.Add(array[i]);
        }
        else
        {
          if (out.Index(array[i].Lower()) == wxNOT_FOUND)
              out.Add(array[i].Lower()); // append only lower-case for the moment
        }
    }
    return out;
}

wxString MakeUniqueString(const wxString& text, const wxString& separator, bool caseSens)
{
    return GetStringFromArray( MakeUniqueArray( GetArrayFromString(text, separator), caseSens ), separator, false );
}

void AppendArray(const wxArrayString& from, wxArrayString& to)
{
    for (size_t i = 0; i < from.GetCount(); ++i)
        to.Add(from[i]);
}

wxString UnixFilename(const wxString& filename, wxPathFormat format)
{
    wxString result = filename;

    if (format == wxPATH_NATIVE)
    {
        if (platform::windows)
            format = wxPATH_WIN;
        else
            format = wxPATH_UNIX;
    }

    // Unc-names always override platform specific settings otherwise they become corrupted
    bool unc_name = result.StartsWith(_T("\\\\"));
    if (format == wxPATH_WIN || unc_name) // wxPATH_WIN == wxPATH_DOS == wxPATH_OS2
    {
        result.Replace(wxT("/"), wxT("\\"));
        while (result.Replace(wxT("\\\\"), wxT("\\")))
            ; // loop for recursive removal of duplicate slashes
        if (unc_name)
            result.Prepend(wxT("\\"));
    }
    else
    {
        result.Replace(wxT("\\"), wxT("/"));
        while (result.Replace(wxT("//"), wxT("/")))
            ; // loop for recursive removal of duplicate slashes
    }

    return result;
}

void QuoteStringIfNeeded(wxString& str)
{
    if ( NeedQuotes(str) )
        str = wxString(_T("\"")) + str + _T("\"");
}

bool NeedQuotes(const wxString &str)
{
    bool hasSpace = str.Find(_T(' ')) != -1;
    bool hasParen = !platform::windows && (str.Find(_T('(')) != -1 || str.Find(_T(')')) != -1);
    return !str.IsEmpty() && str.GetChar(0) != _T('"') && (hasSpace || hasParen);
}

wxString EscapeSpaces(const wxString& str)
{
    wxString ret = str;
    if (!ret.IsEmpty() && ret[0] != _T('"') && ret[0] != _T('\''))
    {
        // TODO: make it faster
        ret.Replace(_T(" "), _T("\\ "));
        ret.Replace(_T("\t"), _T("\\\t"));
    }
    return ret;
}

FileType FileTypeOf(const wxString& filename)
{
    wxString ext = filename.AfterLast(_T('.')).Lower();

    if (ext.IsSameAs(FileFilters::ASM_EXT) ||
        ext.IsSameAs(FileFilters::C_EXT) ||
        ext.IsSameAs(FileFilters::CC_EXT) ||
        ext.IsSameAs(FileFilters::CPP_EXT) ||
        ext.IsSameAs(FileFilters::CXX_EXT) ||
        ext.IsSameAs(FileFilters::CPLPL_EXT) ||
        ext.IsSameAs(FileFilters::S_EXT) ||
        ext.IsSameAs(FileFilters::SS_EXT) ||
        ext.IsSameAs(FileFilters::S62_EXT) ||
        ext.IsSameAs(FileFilters::D_EXT) ||
        ext.IsSameAs(FileFilters::F_EXT) ||
        ext.IsSameAs(FileFilters::F77_EXT) ||
        ext.IsSameAs(FileFilters::F90_EXT) ||
        ext.IsSameAs(FileFilters::F95_EXT) ||
        ext.IsSameAs(FileFilters::FOR_EXT) ||
        ext.IsSameAs(FileFilters::FPP_EXT) ||
        ext.IsSameAs(FileFilters::F03_EXT) ||
        ext.IsSameAs(FileFilters::F08_EXT) ||
        ext.IsSameAs(FileFilters::JAVA_EXT)
       )
        return ftSource;

    else if (ext.IsSameAs(FileFilters::TPP_EXT) ||
             ext.IsSameAs(FileFilters::TCC_EXT)
            )
        return ftTemplateSource;

    else if (ext.IsSameAs(FileFilters::H_EXT) ||
             ext.IsSameAs(FileFilters::HH_EXT) ||
             ext.IsSameAs(FileFilters::HPP_EXT) ||
             ext.IsSameAs(FileFilters::HXX_EXT) ||
             ext.IsSameAs(FileFilters::HPLPL_EXT) ||
             ext.IsSameAs(FileFilters::INL_EXT)
            )
        return ftHeader;

    else if (ext.IsSameAs(FileFilters::CODEBLOCKS_EXT))
        return ftCodeBlocksProject;

    else if (ext.IsSameAs(FileFilters::WORKSPACE_EXT))
        return ftCodeBlocksWorkspace;

    else if (ext.IsSameAs(FileFilters::DEVCPP_EXT))
        return ftDevCppProject;

    else if (ext.IsSameAs(FileFilters::MSVC6_EXT))
        return ftMSVC6Project;

    else if (ext.IsSameAs(FileFilters::MSVC7_EXT))
        return ftMSVC7Project;

    else if (ext.IsSameAs(FileFilters::MSVC10_EXT))
        return ftMSVC10Project;

    else if (ext.IsSameAs(FileFilters::MSVC6_WORKSPACE_EXT))
        return ftMSVC6Workspace;

    else if (ext.IsSameAs(FileFilters::MSVC7_WORKSPACE_EXT))
        return ftMSVC7Workspace;

    else if (ext.IsSameAs(FileFilters::XCODE1_EXT))
        return ftXcode1Project; // Xcode 1.0+ (Mac OS X 10.3)

    else if (ext.IsSameAs(FileFilters::XCODE2_EXT))
        return ftXcode2Project; // Xcode 2.1+ (Mac OS X 10.4)

    else if (ext.IsSameAs(FileFilters::OBJECT_EXT))
        return ftObject;

    else if (ext.IsSameAs(FileFilters::XRCRESOURCE_EXT))
        return ftXRCResource;

    else if (ext.IsSameAs(FileFilters::RESOURCE_EXT))
        return ftResource;

    else if (ext.IsSameAs(FileFilters::RESOURCEBIN_EXT))
        return ftResourceBin;

    else if (ext.IsSameAs(FileFilters::STATICLIB_EXT))
        return ftStaticLib;

    else if (ext.IsSameAs(FileFilters::DYNAMICLIB_EXT))
        return ftDynamicLib;

    else if (ext.IsSameAs(FileFilters::NATIVE_EXT))
        return ftNative;

    else if (ext.IsSameAs(FileFilters::EXECUTABLE_EXT))
        return ftExecutable;

    else if (ext.IsSameAs(FileFilters::XML_EXT))
        return ftXMLDocument;

    else if (ext.IsSameAs(FileFilters::SCRIPT_EXT))
        return ftScript;

    // DrewBoo: Before giving up, see if the ProjectManager
    // considers this extension a source or header
    // TODO (Morten#5#): Do what DrewBoo said: Try removing the above code
    // TODO (Morten#3#): This code should actually be a method of filegroups and masks or alike. So we collect all extension specific things in one place. As of now this would break ABI compatibilty with 08.02 so this should happen later.
    else
    {
        // This code breaks ABI compatibility as noted by (Morten#3#) above.
        // Code commented out by (pecan 2018/04/15). See http://forums.codeblocks.org/index.php/topic,22576.0.html
        // The user can perform an equivalent objective by:
        // 1) Fetching FilesGroupsAndMasks and adding the file extention(s) to file masks in the appropriate group.
        // 2) Using the cbEVT_FILE_ADDED event to set the added file(s) properties (eg., compile and link).

        //ProjectManager *prjMgr = Manager::Get()->GetProjectManager();
        //if ( prjMgr )
        //{
        //    const FilesGroupsAndMasks* fgm = prjMgr->GetFilesGroupsAndMasks();
        //    // Since "ext" var has no "." prefixed, but FilesGropupsAndMasks uses
        //    // dot notation(".ext"), prefix a '.' here.
        //    wxString dotExt = _T(".") + ext;
        //   if (fgm)
        //    {
        //       for (unsigned int i = 0; i != fgm->GetGroupsCount(); ++i)
        //       {
        //            if (fgm->GetGroupName(i) == _T("Sources") && fgm->MatchesMask(dotExt, i))
        //                return ftSource;
        //            if (fgm->GetGroupName(i) == _T("Headers") && fgm->MatchesMask(dotExt, i))
        //                return ftHeader;
        //       }
        //    }
        //}
    }

    return ftOther;
}

wxString cbFindFileInPATH(const wxString &filename)
{
    wxString pathValues;
    wxGetEnv(_T("PATH"), &pathValues);
    if (pathValues.empty())
        return wxEmptyString;

    const wxString &sep = platform::windows ? _T(";") : _T(":");
    wxChar pathSep = wxFileName::GetPathSeparator();
    const wxArrayString &pathArray = GetArrayFromString(pathValues, sep);
    for (size_t i = 0; i < pathArray.GetCount(); ++i)
    {
        if (wxFileExists(pathArray[i] + pathSep + filename))
        {
            if (pathArray[i].AfterLast(pathSep).IsSameAs(_T("bin")))
                return pathArray[i];
        }
    }
    return wxEmptyString;
}

void DoRememberSelectedNodes(wxTreeCtrl* tree, wxArrayString& selectedItemPaths)
{
    wxArrayTreeItemIds items;

    if (tree->GetSelections(items) < 1 )
        return;

    for (size_t i=0; i < items.GetCount(); ++i)
    {
        wxString path = wxEmptyString;
        wxTreeItemId item = items[i];
        while(item.IsOk())
        {
            path = _T("/") + tree->GetItemText(item) + path;
            item = tree->GetItemParent(item);
        }
        if (path != wxEmptyString)
            selectedItemPaths.Add(path);
    }
}

void DoSelectRememberedNode(wxTreeCtrl* tree, const wxTreeItemId& parent, wxString& selectedItemPath)
{
    if (tree && !selectedItemPath.IsEmpty())
    {
        wxString tmpPath;
        wxString folder;
        tmpPath = selectedItemPath;
        int pos = tmpPath.Find(_T('/'));
        while (pos == 0)
        {
            tmpPath = tmpPath.Right(tmpPath.Length() - pos - 1);
            pos = tmpPath.Find(_T('/'));
        }

        folder = tmpPath.Left(pos);
        tmpPath = tmpPath.Right(tmpPath.Length() - pos - 1);
        wxTreeItemId item = parent;
        wxTreeItemIdValue cookie = nullptr;

        while (item.IsOk())
        {
            if (tree->GetItemText(item) != folder)
                item = tree->GetNextSibling(item);
            else
            {
                if (pos < 0)
                {
                    tree->SelectItem(item);
                    break;
                }
                else
                {
                    item = tree->GetNextChild(item, cookie);
                    DoSelectRememberedNode(tree, item, tmpPath);
                }
            }
        }

    }
}

bool DoRememberExpandedNodes(wxTreeCtrl* tree, const wxTreeItemId& parent, wxArrayString& nodePaths, wxString& path)
{
    // remember expanded tree nodes of this tree
    if (!tree || !parent.IsOk())
        return false;

    wxString originalPath = path;
    bool found = false;

    wxTreeItemIdValue cookie = nullptr;

    wxTreeItemId child = tree->GetFirstChild(parent, cookie);
    while (child.IsOk())
    {
        if (tree->ItemHasChildren(child) && tree->IsExpanded(child))
        {
            found = true;
            path << _T("/") << tree->GetItemText(child);
            DoRememberExpandedNodes(tree, child, nodePaths, path);
            nodePaths.Add(path);
            path = originalPath;
        }
        child = tree->GetNextChild(parent, cookie);
    }
    return found;
}

void DoExpandRememberedNode(wxTreeCtrl* tree, const wxTreeItemId& parent, const wxString& path)
{
    if (!path.IsEmpty())
    {
        //Manager::Get()->GetLogManager()->Log(mltDevDebug, path);
        wxString tmpPath;
        tmpPath = path;
        wxString folder;
        int pos = tmpPath.Find(_T('/'));
        while (pos == 0)
        {
            tmpPath = tmpPath.Right(tmpPath.Length() - pos - 1);
            pos = tmpPath.Find(_T('/'));
        }

        if (pos < 0) // no '/'
        {
            folder = tmpPath;
            tmpPath.Clear();
        }
        else
        {
            folder = tmpPath.Left(pos);
            tmpPath = tmpPath.Right(tmpPath.Length() - pos - 1);
        }

        //Manager::Get()->GetLogManager()->Log(mltDevDebug, "%s, %s", folder.c_str(), tmpPath.c_str());

        wxTreeItemIdValue cookie = nullptr;

        wxTreeItemId child = tree->GetFirstChild(parent, cookie);
        while (child.IsOk())
        {
            wxString itemText = tree->GetItemText(child);
            if (itemText.Matches(folder))
            {
                tree->Expand(child);
                DoExpandRememberedNode(tree, child, tmpPath);
                break;
            }
            child = tree->GetNextChild(parent, cookie);
        }
    }
}

void SaveTreeState(wxTreeCtrl* tree, const wxTreeItemId& parent, wxArrayString& nodePaths, wxArrayString& selectedItemPaths)
{
    nodePaths.Clear();
    if (!parent.IsOk() || !tree || !tree->ItemHasChildren(parent) || !tree->IsExpanded(parent))
        return;

    wxString tmp;
    if (!DoRememberExpandedNodes(tree, parent, nodePaths, tmp))
        nodePaths.Add(tmp); // just the tree root

    selectedItemPaths.Clear();
    DoRememberSelectedNodes(tree, selectedItemPaths);
}

void RestoreTreeState(wxTreeCtrl* tree, const wxTreeItemId& parent, wxArrayString& nodePaths, wxArrayString& selectedItemPaths)
{
    if (!parent.IsOk() || !tree)
        return;

    if (nodePaths.GetCount() == 0)
    {
        tree->Collapse(parent);
        return;
    }

    for (unsigned int i = 0; i < nodePaths.GetCount(); ++i)
        DoExpandRememberedNode(tree, parent, nodePaths[i]);

    nodePaths.Clear();
    for (unsigned int i = 0; i < selectedItemPaths.GetCount(); ++i)
        DoSelectRememberedNode(tree, tree->GetRootItem(), selectedItemPaths[i]);

    selectedItemPaths.Clear();
}

bool CreateDirRecursively(const wxString& full_path, int perms)
{
    wxFileName tmp(full_path);
    if (wxDirExists(tmp.GetPath())) // early out, even if full_path is a filename, but the path already exists
        return true;

    wxArrayString dirs;
    wxString currdir;
    currdir = tmp.GetVolume() + tmp.GetVolumeSeparator() + wxFILE_SEP_PATH;
    dirs = tmp.GetDirs();

    for (size_t i = 0; i < dirs.GetCount(); ++i)
    {
        currdir << dirs[i];
        if (!wxDirExists(currdir) && !wxMkdir(currdir, perms))
            return false;
        currdir << wxFILE_SEP_PATH;
    }
    return true;
}

bool CreateDir(const wxString& full_path, int perms)
{
    if (!wxDirExists(full_path) && !wxMkdir(full_path, perms))
        return false;

    return true;
}

wxString ChooseDirectory(wxWindow* parent,
                         const wxString& message,
                         const wxString& initialPath,
                         const wxString& basePath,
                         bool askToMakeRelative, // relative to initialPath
                         bool showCreateDirButton) // where supported
{
    wxDirDialog dlg(parent, message, _T(""),
                    (showCreateDirButton ? wxDD_NEW_DIR_BUTTON : 0) | wxRESIZE_BORDER);
    dlg.SetPath(initialPath);
    PlaceWindow(&dlg);
    if (dlg.ShowModal() != wxID_OK)
        return wxEmptyString;

    wxFileName path(dlg.GetPath());
    if (askToMakeRelative && !basePath.IsEmpty())
    {
        // ask the user if he wants it to be kept as relative
        if (cbMessageBox(_("Keep this as a relative path?"),
                         _("Question"),
                         wxICON_QUESTION | wxYES_NO) == wxID_YES)
        {
            path.MakeRelativeTo(basePath);
        }
    }
    return path.GetFullPath();
}

// Reads a wxString from a file. File must be open. File is closed automatically.
bool cbRead(wxFile& file, wxString& st, wxFontEncoding encoding)
{
    st.Empty();
    if (!file.IsOpened())
        return false;

    int len = file.Length();
    if (!len)
    {
        file.Close();
        return true;
    }

    char* buff = new char[len+1];
    if (!buff) // remark by killerbot : this is useless, since when out of mem --> exception (this is not malloc you know)
    {
        file.Close();
        return false;
    }
    file.Read((void*)buff, len);
    file.Close();
    buff[len]='\0';

    DetectEncodingAndConvert(buff, st, encoding);
    delete [] buff;

    return true;
}

wxString cbReadFileContents(wxFile& file, wxFontEncoding encoding)
{
    wxString st;
    cbRead(file, st, encoding);
    return st;
}

// Writes a wxString to a file. File must be open. File is closed automatically.
bool cbWrite(wxFile& file, const wxString& buff, wxFontEncoding encoding)
{
    bool result = false;
    if (file.IsOpened())
    {
        wxCSConv conv(encoding);
        result = file.Write(buff,conv);
        if (result)
            file.Flush();
        file.Close();
    }
    return result;
}

// Writes a wxString to a file. Takes care of unicode and uses a temporary file
// to save first and then it copies it over the original.
bool cbSaveToFile(const wxString& filename, const wxString& contents, wxFontEncoding encoding,
                  bool bom, bool robust)
{
    return Manager::Get()->GetFileManager()->Save(filename, contents, encoding, bom, robust);
}

// Save a TinyXML document correctly, even if the path contains unicode characters.
bool cbSaveTinyXMLDocument(TiXmlDocument* doc, const wxString& filename)
{
    return TinyXML::SaveDocument(filename, doc);
}

// Return @c str as a proper unicode-compatible string
wxString cbC2U(const char* str)
{
#if wxUSE_UNICODE
    return wxString(str, wxConvUTF8);
#else
    return wxString(str);
#endif
}

// Return multibyte (C string) representation of the string
const wxWX2MBbuf cbU2C(const wxString& str)
{
    if (platform::unicode)
        return str.mb_str(wxConvUTF8);
    else
        return str.mb_str();
}

// Try converting a C-string from different encodings until a possible match is found.
// This tries the following encoding converters (in the same order):
// utf8, system, default and iso8859-1 to iso8859-15
wxFontEncoding DetectEncodingAndConvert(const char* strIn, wxString& strOut, wxFontEncoding possibleEncoding)
{
    wxFontEncoding encoding = possibleEncoding;
    strOut.Clear();

    if (platform::unicode)
    {
        if (possibleEncoding != wxFONTENCODING_UTF16 &&
            possibleEncoding != wxFONTENCODING_UTF16LE &&
            possibleEncoding != wxFONTENCODING_UTF16BE &&
            possibleEncoding != wxFONTENCODING_UTF32 &&
            possibleEncoding != wxFONTENCODING_UTF32LE &&
            possibleEncoding != wxFONTENCODING_UTF32BE)
        {
            // crashes deep in the runtime (windows, at least)
            // if one of the above encodings, hence the guard
            wxCSConv conv(possibleEncoding);
            strOut = wxString(strIn, conv);

            if (strOut.empty())
            {
                // oops! wrong encoding...

                // try utf8 first, if that was not what was asked for
                if (possibleEncoding != wxFONTENCODING_UTF8)
                {
                    encoding = wxFONTENCODING_UTF8;
                    strOut = wxString(strIn, wxConvUTF8);
                }

                // check again: if still not right, try system encoding and then iso8859-1 to iso8859-15
                if (strOut.empty())
                {
                    for (int i = wxFONTENCODING_SYSTEM; i < wxFONTENCODING_ISO8859_MAX; ++i)
                    {
                        encoding = (wxFontEncoding)i;
                        // skip if same as what was asked
                        if (encoding == possibleEncoding)
                            continue;

                        strOut = wxString(strIn, (encoding == wxFONTENCODING_DEFAULT) ? wxConvLocal : wxCSConv(encoding));
                        if (!strOut.empty())
                            break; // got it!
                    }
                }
            }
        }
        else
        {
            strOut = (const wxChar*) strIn;
        }
    }
    else
    {
        strOut = (const wxChar*) strIn;
    }
    return encoding;
}

wxString GetEOLStr(int eolMode)
{
    if (eolMode == -1)
    {
        static const int defEOL = platform::windows ? wxSCI_EOL_CRLF : wxSCI_EOL_LF;
        eolMode = Manager::Get()->GetConfigManager(wxT("editor"))->ReadInt(wxT("/eol/eolmode"), defEOL);
        if (eolMode == 3) // auto-detect EOL
            eolMode = defEOL;
    }
    switch (eolMode)
    {
      case wxSCI_EOL_CR:
          return wxT("\r");
      case wxSCI_EOL_LF:
          return wxT("\n");
      default: // wxSCI_EOL_CRLF
          return wxT("\r\n");
    }
}

wxString URLEncode(const wxString &str) // not sure this is 100% standards compliant, but I hope so
{
    wxString ret;
    wxString t;
    for (unsigned int i = 0; i < str.length(); ++i)
    {
        wxChar c = str[i];
        if (  (c >= _T('A') && c <= _T('Z'))
           || (c >= _T('a') && c <= _T('z'))
           || (c >= _T('0') && c <= _T('9'))
           || (c == _T('.'))
           || (c == _T('-'))
           || (c == _T('_')) )

            ret.Append(c);
        else if (c == _T(' '))
            ret.Append(_T('+'));
        else
        {
            t.sprintf(_T("%%%02X"), (unsigned int) c);
            ret.Append(t);
        }
    }
    return ret;
}

/** Adds support for backtick'd expressions under Windows. */

cbBackticksMap m_Backticks; // all calls share the same cache

wxString cbExpandBackticks(wxString& str) // backticks are written in-place to str
{
    wxString ret;

    // this function is not windows-only anymore because we parse the backticked command's output
    // for compiler/linker search dirs

    size_t start = str.find(_T('`'));
    if (start == wxString::npos)
        return ret; // no backticks here
    size_t end = str.find(_T('`'), start + 1);
    if (end == wxString::npos)
        return ret; // no ending backtick; error?

    while (start != wxString::npos && end != wxString::npos)
    {
        wxString cmd = str.substr(start + 1, end - start - 1);
        cmd.Trim(true);
        cmd.Trim(false);
        if (cmd.IsEmpty())
            break;

        wxString bt;
        cbBackticksMap::iterator it = m_Backticks.find(cmd);
        if (it != m_Backticks.end()) // in the cache :)
            bt = it->second;
        else
        {
            LogManager *log = Manager::Get()->GetLogManager();
            log->DebugLog(wxString::Format("Caching result of `%s`", cmd));

            wxString fullCmd;

            if (platform::windows)
                fullCmd = "cmd /c " + cmd;
            else
            {
                ConfigManager *conf = Manager::Get()->GetConfigManager(_T("app"));
                const wxString shell = conf->Read(_T("/console_shell"), DEFAULT_CONSOLE_SHELL);
                fullCmd = cmd;
                fullCmd.Replace("'", "\\'");
                fullCmd = shell + " '" + fullCmd + "'";
            }
            wxArrayString output;
            const long exitCode = wxExecute(fullCmd, output, wxEXEC_NODISABLE);
            bt = GetStringFromArray(output, _T(" "), false);
            // add it in the cache
            m_Backticks[cmd] = bt;
            log->DebugLog(wxString::Format("Cached: '%s' (full cmd: '%s' exit code: %d)", bt,
                                           fullCmd, int(exitCode)));
        }
        ret << bt << _T(' ');
        str = str.substr(0, start) + bt + str.substr(end + 1, wxString::npos);

        // find next occurrence
        start = str.find(_T('`'));
        end = str.find(_T('`'), start + 1);
    }

    return ret; // return a list of the replaced expressions
}

void cbClearBackticksCache()
{
    Manager::Get()->GetLogManager()->DebugLog("Cached: cleared!");
    m_Backticks.clear();
}

const cbBackticksMap& cbGetBackticksCache()
{
    return m_Backticks;
}

wxMenu* CopyMenu(wxMenu* mnu, bool with_accelerators)
{
    if (!mnu || mnu->GetMenuItemCount() < 1)
        return nullptr;
    wxMenu* theMenu = new wxMenu();

    for (size_t i = 0; i < mnu->GetMenuItemCount();++i)
    {
        wxMenuItem* tmpItem = mnu->FindItemByPosition(i);
        wxMenuItem* theItem = new wxMenuItem(nullptr,
                                             tmpItem->GetId(),
                                             with_accelerators?tmpItem->GetItemLabel():tmpItem->GetItemLabelText(),
                                             tmpItem->GetHelp(),
                                             tmpItem->GetKind(),
                                             CopyMenu(tmpItem->GetSubMenu()));
        theMenu->Append(theItem);
    }
    return theMenu;
}

bool IsWindowReallyShown(wxWindow* win)
{
    while (win && win->IsShown())
    {
        win = win->GetParent();
        if (!win)
            return true;
    }
    return false;
}

bool NormalizePath(wxFileName& f,const wxString& base)
{
    bool result = true;
//    if (!f.IsAbsolute())
    {
        f.Normalize(wxPATH_NORM_DOTS | wxPATH_NORM_TILDE | wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG | wxPATH_NORM_SHORTCUT, base);
        result = f.IsOk();
    }
    return result;
}

// Checks whether 'suffix' could be a suffix of 'path' and therefore represents
// the same path. This is used to check whether a relative path could represent
// the same path as absolute path. For instance, for
// suffix = sdk/globals.cpp
// path = /home/user/codeblocks/trunk/src/sdk/globals.cpp
// it returns true. The function expects that 'path' is normalized and compares
// 'path' with 'suffix' starting from the end of the path. When it reaches .. in
// 'suffix' it gives up (there is no way to check relative filename names
// exactly) and if the path compared so far is identical, it returns true
bool IsSuffixOfPath(wxFileName const & suffix, wxFileName const & path)
{
    if (path.GetFullName() != suffix.GetFullName())
    {
        return false;
    }

    wxArrayString suffixDirArray = suffix.GetDirs();
    wxArrayString pathDirArray = path.GetDirs();

    int j = pathDirArray.GetCount() - 1;
    for (int i = suffixDirArray.GetCount() - 1; i >= 0; i--)
    {
        // skip paths like /./././ and ////
        if (suffixDirArray[i] == _T(".") || suffixDirArray[i] == _T(""))
            continue;

        // suffix has more directories than path - cannot represent the same path
        if (j < 0)
            return false;

        // suffix contains ".." - from now on we cannot precisely determine
        // whether suffix and path match - we assume that they do
        if (suffixDirArray[i] == _T(".."))
            return true;
        // the corresponding directories of the two paths differ
        else if (suffixDirArray[i] != pathDirArray[j])
            return false;

        j--;
    }

    if (suffix.IsAbsolute() && (j >= 0 || !(suffix.GetVolume().IsSameAs(path.GetVolume(), false))) )
        return false;

    // 'suffix' is a suffix of 'path'
    return true;
}

bool cbResolveSymLinkedDirPath(wxString& dirpath)
{
#ifdef _WIN32
    wxUnusedVar(dirpath);
    return false;
#else
    if (dirpath.empty())
        return false;
    if (dirpath.Last() == wxFILE_SEP_PATH)
        dirpath.RemoveLast();

    struct stat fileStats;
    if (lstat(dirpath.mb_str(wxConvUTF8), &fileStats) != 0)
        return false;

    // If the path is a symbolic link, then try to resolve it.
    // This is needed to prevent infinite loops, when a folder is pointing to itself or its parent folder.
    if (S_ISLNK(fileStats.st_mode))
    {
        char buffer[4096];
        int result = readlink(dirpath.mb_str(wxConvUTF8), buffer, WXSIZEOF(buffer) - 1);
        if (result != -1)
        {
            buffer[result] = '\0'; // readlink() doesn't NUL-terminate the buffer
            wxString pathStr(buffer, wxConvUTF8);
            wxFileName fileName = wxFileName::DirName(pathStr);

            // If this is a relative symbolic link, we need to make it absolute.
            if (!fileName.IsAbsolute())
            {
                wxFileName dirNamePath;
                if (dirpath.Last() == wxFILE_SEP_PATH)
                    dirNamePath = wxFileName::DirName(dirpath);
                else
                    dirNamePath = wxFileName::DirName(dirpath + wxFILE_SEP_PATH);
                dirNamePath.RemoveLastDir();
                // Make the new filename absolute relative to the parent folder.
                fileName.MakeAbsolute(dirNamePath.GetFullPath());
            }

            wxString fullPath = fileName.GetFullPath();
            if (!fullPath.empty() && fullPath.Last() == wxT('.')) // this case should be handled because of a bug in wxWidgets
                fullPath.RemoveLast();
            if (fullPath.length() > 1 && fullPath.Last() == wxFILE_SEP_PATH)
                fullPath.RemoveLast();
            dirpath = fullPath;
            return true;
        }
    }

    return false;
#endif // _WIN32
}

wxString cbResolveSymLinkedDirPathRecursive(wxString dirpath)
{
    while (cbResolveSymLinkedDirPath(dirpath))
        ;
    return dirpath;
}

// function to check the common controls version
#ifdef __WXMSW__
#include <windows.h>
#include <shlwapi.h>
bool UsesCommonControls6()
{
    bool result = false;
    HINSTANCE hinstDll;
    hinstDll = LoadLibrary(_T("comctl32.dll"));
    if (hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

        if (pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if (SUCCEEDED(hr))
               result = dvi.dwMajorVersion == 6;
        }

        FreeLibrary(hinstDll);
    }
    return result;
}
#else
bool UsesCommonControls6()
{
    // for non-windows platforms, return true
    // as this is only used for knowing if bitmaps support transparency or not
    return true;
}
#endif

static void cbLoadImageFromFS(wxImage &image, const wxString& filename, wxBitmapType bitmapType,
                              wxFileSystem& fs)
{
    // cache this, can't change while we 're running :)
    static bool oldCommonControls = !UsesCommonControls6();

    wxFSFile* f = fs.OpenFile(filename);
    if (f)
    {
        wxInputStream* is = f->GetStream();
        image.LoadFile(*is, bitmapType);
        delete f;
    }
    if (oldCommonControls && image.HasAlpha())
        image.ConvertAlphaToMask();
}

wxBitmap cbLoadBitmap(const wxString& filename, wxBitmapType bitmapType, wxFileSystem* fs)
{
    wxImage im;
    if (fs)
        cbLoadImageFromFS(im, filename, bitmapType, *fs);
    else
    {
        wxFileSystem defaultFS;
        cbLoadImageFromFS(im, filename, bitmapType, defaultFS);
    }

    if (!im.IsOk())
        return wxNullBitmap;

    return wxBitmap(im);
}

#if wxCHECK_VERSION(3, 1, 6)
wxBitmapBundle cbLoadBitmapBundle(const wxString& prefix, const wxString& filename, int minSize,
                                  wxBitmapType bitmapType, wxFileSystem* fs)
{
    static const int imageSize[] = {16, 20, 24, 28, 32, 40, 48, 56, 64};

    wxVector<wxBitmap> bitmaps;
    for (const int sz : imageSize)
    {
        // Do not load bitmaps smaller than needed
        if (sz < minSize)
            continue;

        const wxString pngName(prefix + wxString::Format("%dx%d/", sz, sz) + filename);
        const wxBitmap bmp(cbLoadBitmap(pngName, bitmapType, fs));
        if (bmp.IsOk())
            bitmaps.push_back(bmp);
        else
            Manager::Get()->GetLogManager()->DebugLog(wxString::Format("cbLoadBitmapBundle: Cannot load bitmap '%s'", pngName));
    }

    if (!bitmaps.empty())
        return wxBitmapBundle::FromBitmaps(bitmaps);

    // Some images do not provide the per-size directory layout (or only a single size exists).
    // In this case fall back to a single bitmap so that the caller still gets a valid bundle.
    const wxBitmap fallback(cbLoadBitmap(prefix + filename, bitmapType, fs));
    if (fallback.IsOk())
        return wxBitmapBundle::FromBitmap(fallback);

    return wxBitmapBundle();
}

wxBitmapBundle cbLoadBitmapBundleFromSVG(const wxString& filename, const wxSize& size, wxFileSystem* fs)
{
    wxBitmapBundle bundle;

#ifdef wxHAS_SVG
    wxFileSystem defaultFS;
    if (!fs)
        fs = &defaultFS;

    wxFSFile* f = fs->OpenFile(filename);
    if (f)
    {
        wxInputStream* is = f->GetStream();
        if (is->IsOk())
        {
            const size_t dataSize = is->GetSize();
            if (dataSize)
            {
                wxByte *data = new wxByte[dataSize];
                if (is->ReadAll(data, dataSize))
                    bundle = wxBitmapBundle::FromSVG(data, dataSize, size);

                delete [] data;
            }
        }

        delete f;
    }

    if (!bundle.IsOk())
        Manager::Get()->GetLogManager()->DebugLog(wxString::Format("cbLoadBitmapBundleFromSVG: Cannot load '%s'", filename));

#else
#warning The port does not provide raw bitmap accessvia wxPixelData, so SVG loading will fail
#endif

    return bundle;
}
#endif

wxBitmap cbLoadBitmapScaled(const wxString& filename, wxBitmapType bitmapType, double scaleFactor,
                            wxFileSystem* fs)
{

    wxImage im;
    if (fs)
        cbLoadImageFromFS(im, filename, bitmapType, *fs);
    else
    {
        wxFileSystem defaultFS;
        cbLoadImageFromFS(im, filename, bitmapType, defaultFS);
    }

    if (!im.IsOk())
        return wxNullBitmap;

#if defined(__WXOSX__) || (defined(__WXGTK3__) && wxCHECK_VERSION(3, 1, 2))
    return wxBitmap(im, -1, scaleFactor);
#else
    (void)scaleFactor;
    return wxBitmap(im);
#endif // defined(__WXOSX__) || (defined(__WXGTK3__) && wxCHECK_VERSION(3, 1, 2))
}

wxBitmap cbScaleBitmapToFit(const wxBitmap& bitmap, const wxSize& maxSize)
{
    if (!bitmap.IsOk() || maxSize.x <= 0 || maxSize.y <= 0)
        return bitmap;

    const wxSize size = bitmap.GetSize();
    if (size.x <= maxSize.x && size.y <= maxSize.y)
        return bitmap;

    const double scale = std::min(static_cast<double>(maxSize.x) / size.x,
                                  static_cast<double>(maxSize.y) / size.y);
    const int newWidth = std::max(1, static_cast<int>(size.x * scale));
    const int newHeight = std::max(1, static_cast<int>(size.y * scale));

    wxImage image = bitmap.ConvertToImage();
    if (!image.IsOk())
        return bitmap;

    return wxBitmap(image.Scale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH));
}

double cbGetContentScaleFactor(const wxWindow &window)
{
#if wxCHECK_VERSION(3, 1, 4)
    return window.GetDPIScaleFactor();
#else
    return window.GetContentScaleFactor();
#endif
}

#ifdef __WXGTK__
// GTK 2 doesn't support scaling.
// GTK 3 supports scaling, but doesn't support fractional values.
// In both cases we need to make up our on scaling value.
// For other platforms the value returned by GetContentScalingFactor seems adequate.
double cbGetActualContentScaleFactor(cb_unused const wxWindow &window)
{
    // It is possible to use the window to find a display, but unfortunately this doesn't work well,
    // because we call this function mostly on windows which haven't been shown. This leads to
    // warnings in the log about ClientToScreen failures.
    // If there are problems on multi-monitor setups we should think about some other solution. :(
    const wxSize ppi = wxGetDisplayPPI();
    return ppi.y / 96.0;
}
#else // __WXGTK__
double cbGetActualContentScaleFactor(const wxWindow &window)
{
    return cbGetContentScaleFactor(window);
}
#endif // __WXGTK__

int cbFindMinSize(int targetSize, const int possibleSize[], int numWidths)
{
    int selected = possibleSize[0];
    for (int ii = 0; ii < numWidths; ++ii)
    {
        if (possibleSize[ii] <= targetSize)
            selected = possibleSize[ii];
        else
            break;
    }
    return selected;
}

int cbFindMinSize16to64(int targetSize)
{
    const int sizes[] = { 16, 20, 24, 28, 32, 40, 48, 56, 64 };
    return cbFindMinSize(targetSize, sizes, cbCountOf(sizes));
}

std::unique_ptr<wxImageList> cbMakeScaledImageList(int size, double scaleFactor, int &outActualSize)
{
#ifdef __WXMSW__
    outActualSize = size;
#else
    outActualSize = wxRound(size/scaleFactor);
#endif

    return std::unique_ptr <wxImageList> (new wxImageList(outActualSize, outActualSize));
}

bool cbAddBitmapToImageList(wxImageList &list, const wxBitmap &bitmap, int size, cb_unused int listSize,
                            cb_unused double scaleFactor)
{
    if (bitmap.IsOk())
    {
        list.Add(bitmap);
        return true;
    }
    else
    {
        wxBitmap missingBitmap(size, size);
        // Draw red square image
        wxMemoryDC dc;
        dc.SelectObject(missingBitmap);
        dc.SetBrush(*wxRED_BRUSH);
        dc.DrawRectangle(0, 0, size, size);
        dc.SelectObject(wxNullBitmap);
        list.Add(missingBitmap);
        return false;
    }
}

cbAppTheme cbGetAppTheme()
{
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("app"));
    if (!cfg)
        return cbAppThemeLight;

    const wxString theme = cfg->Read(_T("/environment/theme"), _T("light"));
    return theme.IsSameAs(_T("dark"), false) ? cbAppThemeDark : cbAppThemeLight;
}

bool cbIsAppThemeDark()
{
    return cbGetAppTheme() == cbAppThemeDark;
}

bool cbIsDarkTheme()
{
    // The explicit application theme setting takes precedence over the system appearance,
    // because the application applies it to its own windows.
    ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("app"));
    if (cfg)
        return cbIsAppThemeDark();

    // Fall back to the system appearance if the configuration is not available yet.
    bool isDarkTheme = false;
#if wxCHECK_VERSION(3, 1, 3)
    isDarkTheme = wxSystemSettings::GetAppearance().IsDark();
#else
    // Taken from wxSystemAppearance::IsUsingDarkBackground in wxWidgets...
    const wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    const wxColour fg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

    const double fgLuminance = (0.299 * fg.Red() + 0.587 * fg.Green() + 0.114 * fg.Blue()) / 255.0;
    const double bgLuminance = (0.299 * bg.Red() + 0.587 * bg.Green() + 0.114 * bg.Blue()) / 255.0;
    // The threshold here is rather arbitrary, but it seems that using just
    // inequality would be wrong as it could result in false positives.
    isDarkTheme = ((fgLuminance - bgLuminance) > 0.2);
#endif // wxCHECK_VERSION(3, 1, 3)

    return isDarkTheme;
}

#ifdef __WXMSW__
namespace
{
typedef HRESULT (WINAPI *DwmSetWindowAttributeFunction)(HWND, DWORD, LPCVOID, DWORD);
typedef HRESULT (WINAPI *SetWindowThemeFunction)(HWND, LPCWSTR, LPCWSTR);
typedef int (WINAPI *SetPreferredAppModeFunction)(int);
typedef BOOL (WINAPI *AllowDarkModeForWindowFunction)(HWND, BOOL);
typedef void (WINAPI *FlushMenuThemesFunction)();

DwmSetWindowAttributeFunction GetDwmSetWindowAttributeFunction()
{
    static const DwmSetWindowAttributeFunction function = []()
    {
        const HMODULE library = ::LoadLibraryW(L"dwmapi.dll");
        return library ? reinterpret_cast<DwmSetWindowAttributeFunction>(
                             ::GetProcAddress(library, "DwmSetWindowAttribute")) : nullptr;
    }();
    return function;
}

SetWindowThemeFunction GetSetWindowThemeFunction()
{
    static const SetWindowThemeFunction function = []()
    {
        const HMODULE library = ::LoadLibraryW(L"uxtheme.dll");
        return library ? reinterpret_cast<SetWindowThemeFunction>(
                             ::GetProcAddress(library, "SetWindowTheme")) : nullptr;
    }();
    return function;
}

SetPreferredAppModeFunction GetSetPreferredAppModeFunction()
{
    static const SetPreferredAppModeFunction function = []()
    {
        const HMODULE library = ::LoadLibraryW(L"uxtheme.dll");
        return library ? reinterpret_cast<SetPreferredAppModeFunction>(
                             ::GetProcAddress(library, reinterpret_cast<LPCSTR>(135))) : nullptr;
    }();
    return function;
}

AllowDarkModeForWindowFunction GetAllowDarkModeForWindowFunction()
{
    static const AllowDarkModeForWindowFunction function = []()
    {
        const HMODULE library = ::LoadLibraryW(L"uxtheme.dll");
        return library ? reinterpret_cast<AllowDarkModeForWindowFunction>(
                             ::GetProcAddress(library, reinterpret_cast<LPCSTR>(133))) : nullptr;
    }();
    return function;
}

FlushMenuThemesFunction GetFlushMenuThemesFunction()
{
    static const FlushMenuThemesFunction function = []()
    {
        const HMODULE library = ::LoadLibraryW(L"uxtheme.dll");
        return library ? reinterpret_cast<FlushMenuThemesFunction>(
                             ::GetProcAddress(library, reinterpret_cast<LPCSTR>(136))) : nullptr;
    }();
    return function;
}

DWORD GetWindowsBuildNumber()
{
    // RtlGetVersion reports the real build number even when the application has no
    // Windows 10 compatibility manifest (GetVersionEx would lie in that case).
    static const DWORD buildNumber = []() -> DWORD
    {
        typedef LONG (WINAPI *RtlGetVersionFunction)(OSVERSIONINFOW*);
        const HMODULE ntdll = ::GetModuleHandleW(L"ntdll.dll");
        if (!ntdll)
            return 0;

        const RtlGetVersionFunction rtlGetVersion =
            reinterpret_cast<RtlGetVersionFunction>(::GetProcAddress(ntdll, "RtlGetVersion"));
        if (!rtlGetVersion)
            return 0;

        OSVERSIONINFOW info = {};
        info.dwOSVersionInfoSize = sizeof(info);
        if (rtlGetVersion(&info) != 0)
            return 0;

        return info.dwBuildNumber;
    }();
    return buildNumber;
}

bool IsWindowsDarkModeAvailable()
{
    // The undocumented dark-mode entry points are available since Windows 10 1809.
    return GetWindowsBuildNumber() >= 17763;
}

// Windows 11 24H2/25H2 (build 26200 and later) ships the "DarkMode_DarkTheme" theme
// which paints the standard common controls (including combo boxes and text fields)
// completely dark. Older versions only have partial themes for some controls.
bool HasWindowsDarkTheme()
{
    return GetWindowsBuildNumber() >= 26200;
}

const wchar_t* GetDarkNativeThemeName(const wxString& className)
{
    if (HasWindowsDarkTheme())
        return L"DarkMode_DarkTheme";

    if (className.Contains("ComboBox"))
        return L"CFD";
    if (className.Contains("TextCtrl") || className.Contains("SearchCtrl") ||
        className == "Edit")
        return L"CFD";

    return L"DarkMode_Explorer";
}

void ApplyNativeThemeToHandle(HWND hwnd, const wchar_t* themeName, bool dark)
{
    if (!hwnd)
        return;

    const SetWindowThemeFunction setWindowTheme = GetSetWindowThemeFunction();
    if (!setWindowTheme)
        return;

    const HRESULT result = setWindowTheme(hwnd, dark ? themeName : L"Explorer", nullptr);
    (void)result;

    // Some controls (notably buttons, tooltips and controls with scroll bars) only
    // switch their colours after receiving WM_THEMECHANGED.
    ::SendMessageW(hwnd, WM_THEMECHANGED, 0, 0);
}

void FillRectWithColour(HDC dc, const RECT& rect, const wxColour& colour)
{
    const HBRUSH brush = ::CreateSolidBrush(RGB(colour.Red(), colour.Green(), colour.Blue()));
    if (!brush)
        return;

    ::FillRect(dc, &rect, brush);
    ::DeleteObject(brush);
}

void ApplyNativeThemeToComboBoxInternals(HWND hwnd, bool dark)
{
    COMBOBOXINFO info = {};
    info.cbSize = sizeof(info);
    if (!::GetComboBoxInfo(hwnd, &info))
        return;

    const wchar_t* themeName = dark ? GetDarkNativeThemeName("Edit") : L"Explorer";

    // The edit part of a drop-down combo box is a separate native control which
    // keeps its light background unless it is themed explicitly.
    if (info.hwndItem && info.hwndItem != hwnd)
    {
        wchar_t className[64] = {};
        ::GetClassNameW(info.hwndItem, className, WXSIZEOF(className));
        if (wxString(className).IsSameAs("Edit", false))
            ApplyNativeThemeToHandle(info.hwndItem, themeName, dark);
    }

    if (info.hwndList && info.hwndList != hwnd)
        ApplyNativeThemeToHandle(info.hwndList, dark ? L"DarkMode_Explorer" : L"Explorer", dark);
}

bool IsNativeClass(HWND hwnd, const wchar_t* name)
{
    if (!hwnd)
        return false;

    wchar_t className[64] = {};
    ::GetClassNameW(hwnd, className, WXSIZEOF(className));
    return wxString(className).IsSameAs(name, false);
}

bool IsEditOrComboBoxHandle(HWND hwnd)
{
    return IsNativeClass(hwnd, L"Edit") || IsNativeClass(hwnd, L"ComboBox");
}

bool IsEtchedStaticLine(HWND hwnd)
{
    // wxStaticLine is a native STATIC control with the SS_ETCHEDHORZ/SS_ETCHEDVERT
    // style. Windows always draws it with the light 3D border colours, even when the
    // application dark mode is enabled, so it has to be painted by us.
    if (!IsNativeClass(hwnd, L"Static"))
        return false;

    const LONG_PTR style = ::GetWindowLongPtrW(hwnd, GWL_STYLE);
    const LONG_PTR type = style & SS_TYPEMASK;
    // wxStaticLine uses SS_GRAYRECT | SS_SUNKEN. The SS_ETCHED* variants are accepted
    // as well, in case the control is created by native code outside wxWidgets.
    return type == SS_GRAYRECT ||
           type == SS_ETCHEDHORZ || type == SS_ETCHEDVERT || type == SS_ETCHEDFRAME;
}

HBRUSH GetDarkControlBackgroundBrush()
{
    // The brush is intentionally never destroyed: it is process-wide and is returned
    // to Windows from WM_CTLCOLOR handlers for the whole lifetime of the application.
    static const HBRUSH brush = ::CreateSolidBrush(
        RGB(cbTheme::Panel().Red(), cbTheme::Panel().Green(), cbTheme::Panel().Blue()));
    return brush;
}

void DrawDarkHeaderSortArrow(HDC dc, const RECT& rect, bool ascending)
{
    const int x = rect.right - 9;
    const int y = (rect.top + rect.bottom) / 2;

    POINT points[3];
    if (ascending)
    {
        points[0] = POINT{ x - 4, y + 2 };
        points[1] = POINT{ x + 4, y + 2 };
        points[2] = POINT{ x, y - 2 };
    }
    else
    {
        points[0] = POINT{ x - 4, y - 2 };
        points[1] = POINT{ x + 4, y - 2 };
        points[2] = POINT{ x, y + 2 };
    }

    const HBRUSH brush = ::CreateSolidBrush(
        RGB(cbTheme::MutedText().Red(), cbTheme::MutedText().Green(), cbTheme::MutedText().Blue()));
    const HPEN pen = ::CreatePen(PS_SOLID, 1,
        RGB(cbTheme::MutedText().Red(), cbTheme::MutedText().Green(), cbTheme::MutedText().Blue()));
    if (!brush || !pen)
    {
        if (brush)
            ::DeleteObject(brush);
        if (pen)
            ::DeleteObject(pen);
        return;
    }

    const HGDIOBJ oldBrush = ::SelectObject(dc, brush);
    const HGDIOBJ oldPen = ::SelectObject(dc, pen);
    ::Polygon(dc, points, 3);
    ::SelectObject(dc, oldPen);
    ::SelectObject(dc, oldBrush);
    ::DeleteObject(pen);
    ::DeleteObject(brush);
}

void DrawDarkHeaderItem(LPNMCUSTOMDRAW customDraw)
{
    HDC dc = customDraw->hdc;
    RECT itemRect = customDraw->rc;

    const bool hot = (customDraw->uItemState & CDIS_HOT) != 0;
    const bool selected = (customDraw->uItemState & CDIS_SELECTED) != 0;
    const wxColour& background = selected ? cbTheme::SelectionAlt()
                                : (hot ? cbTheme::Selection() : cbTheme::Window());
    FillRectWithColour(dc, itemRect, background);

    RECT border = itemRect;
    border.top = border.bottom - 1;
    FillRectWithColour(dc, border, cbTheme::Border());
    border = itemRect;
    border.left = border.right - 1;
    FillRectWithColour(dc, border, cbTheme::Border());

    HDITEMW item = {};
    item.mask = HDI_TEXT | HDI_FORMAT;
    wchar_t text[256] = {};
    item.pszText = text;
    item.cchTextMax = WXSIZEOF(text);

    if (::SendMessageW(customDraw->hdr.hwndFrom, HDM_GETITEMW,
                       static_cast<WPARAM>(customDraw->dwItemSpec),
                       reinterpret_cast<LPARAM>(&item)))
    {
        RECT textRect = itemRect;
        textRect.left += 7;
        textRect.right -= 20;

        const HFONT font = reinterpret_cast<HFONT>(
            ::SendMessageW(customDraw->hdr.hwndFrom, WM_GETFONT, 0, 0));
        const HGDIOBJ oldFont = font ? ::SelectObject(dc, font) : nullptr;
        const int oldMode = ::SetBkMode(dc, TRANSPARENT);
        const COLORREF oldColour = ::SetTextColor(dc,
            RGB(cbTheme::Text().Red(), cbTheme::Text().Green(), cbTheme::Text().Blue()));

        ::DrawTextW(dc, item.pszText, -1, &textRect,
                    DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

        ::SetTextColor(dc, oldColour);
        ::SetBkMode(dc, oldMode);
        if (oldFont)
            ::SelectObject(dc, oldFont);
    }

    if (item.fmt & HDF_SORTUP)
        DrawDarkHeaderSortArrow(dc, itemRect, true);
    else if (item.fmt & HDF_SORTDOWN)
        DrawDarkHeaderSortArrow(dc, itemRect, false);
}

LRESULT HandleDarkHeaderCustomDraw(LPNMHDR notification)
{
    LPNMCUSTOMDRAW customDraw = reinterpret_cast<LPNMCUSTOMDRAW>(notification);
    switch (customDraw->dwDrawStage)
    {
        case CDDS_PREPAINT:
            {
                // The native header does not have a dark uxtheme variant, so it has to be
                // painted completely by us. This is used by all report/list controls,
                // including the Build messages list. customDraw->rc only covers the items,
                // so the empty area after the last column has to be filled separately.
                RECT headerRect = {};
                ::GetClientRect(customDraw->hdr.hwndFrom, &headerRect);
                FillRectWithColour(customDraw->hdc, headerRect, cbTheme::Window());
                return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
            }

        case CDDS_ITEMPREPAINT:
            DrawDarkHeaderItem(customDraw);
            return CDRF_SKIPDEFAULT;

        case CDDS_POSTPAINT:
            {
                // The header control paints the area after the last column itself using
                // its native (lighter) filler brush, after the custom draw notifications.
                // Repaint that strip with the window background.
                RECT headerRect = {};
                ::GetClientRect(customDraw->hdr.hwndFrom, &headerRect);

                const int itemCount = static_cast<int>(
                    ::SendMessageW(customDraw->hdr.hwndFrom, HDM_GETITEMCOUNT, 0, 0));
                int lastItemRight = headerRect.left;
                if (itemCount > 0)
                {
                    RECT lastItem = {};
                    if (::SendMessageW(customDraw->hdr.hwndFrom, HDM_GETITEMRECT,
                                       static_cast<WPARAM>(itemCount - 1),
                                       reinterpret_cast<LPARAM>(&lastItem)))
                        lastItemRight = lastItem.right;
                }

                RECT filler = headerRect;
                filler.left = lastItemRight;
                if (filler.left < filler.right)
                    FillRectWithColour(customDraw->hdc, filler, cbTheme::Window());
                return CDRF_DODEFAULT;
            }

        default:
            break;
    }

    return CDRF_DODEFAULT;
}

LRESULT HandleDarkTrackBarCustomDraw(LPNMHDR notification)
{
    LPNMCUSTOMDRAW customDraw = reinterpret_cast<LPNMCUSTOMDRAW>(notification);
    if (customDraw->dwDrawStage == CDDS_PREPAINT)
        return CDRF_NOTIFYITEMDRAW;

    if (customDraw->dwDrawStage == CDDS_ITEMPREPAINT &&
        customDraw->dwItemSpec == TBCD_CHANNEL)
    {
        // The Win32 trackbar has no dark theme: the channel is always drawn with the
        // light system colours. customDraw->rc already contains the channel rectangle
        // (TBM_GETCHANNELRECT fails for these themed controls), and the thumb is sent
        // as a later item, so it is drawn on top of this fill and keeps its accent colour.
        FillRectWithColour(customDraw->hdc, customDraw->rc, cbTheme::Border());
        return CDRF_SKIPDEFAULT;
    }

    return CDRF_DODEFAULT;
}

void PaintDarkControlFrame(HWND hwnd)
{
    HDC dc = ::GetWindowDC(hwnd);
    if (!dc)
        return;

    RECT windowRect = {};
    RECT clientRect = {};
    if (!::GetWindowRect(hwnd, &windowRect) || !::GetClientRect(hwnd, &clientRect))
    {
        ::ReleaseDC(hwnd, dc);
        return;
    }

    POINT clientOrigin = { 0, 0 };
    ::ClientToScreen(hwnd, &clientOrigin);
    clientOrigin.x -= windowRect.left;
    clientOrigin.y -= windowRect.top;

    const int width = windowRect.right - windowRect.left;
    const int height = windowRect.bottom - windowRect.top;
    const RECT frame = { 0, 0, width, height };
    const RECT client = { clientOrigin.x, clientOrigin.y,
                          clientOrigin.x + (clientRect.right - clientRect.left),
                          clientOrigin.y + (clientRect.bottom - clientRect.top) };

    // Only the non-client strips need to be filled here; the client area is painted
    // through the dark WM_CTLCOLOR brush.
    RECT strip = { 0, 0, width, client.top };
    FillRectWithColour(dc, strip, cbTheme::Panel());
    strip = RECT{ 0, client.bottom, width, height };
    FillRectWithColour(dc, strip, cbTheme::Panel());
    strip = RECT{ 0, client.top, client.left, client.bottom };
    FillRectWithColour(dc, strip, cbTheme::Panel());
    strip = RECT{ client.right, client.top, width, client.bottom };
    FillRectWithColour(dc, strip, cbTheme::Panel());

    const HBRUSH border = ::CreateSolidBrush(
        RGB(cbTheme::Border().Red(), cbTheme::Border().Green(), cbTheme::Border().Blue()));
    if (border)
    {
        ::FrameRect(dc, &frame, border);
        ::DeleteObject(border);
    }

    ::ReleaseDC(hwnd, dc);
}

void InstallDarkThemeSubclass(HWND hwnd);
void ApplyNativeThemeToChildren(HWND parent, bool dark);

LRESULT CALLBACK DarkThemeSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                       UINT_PTR idSubclass, DWORD_PTR refData)
{
    wxUnusedVar(idSubclass);
    wxUnusedVar(refData);

    if (cbIsAppThemeDark())
    {
        switch (msg)
        {
            case WM_CTLCOLORSTATIC:
            case WM_CTLCOLOREDIT:
            case WM_CTLCOLORLISTBOX:
                {
                    // wxWidgets 3.2 paints disabled single-line text controls and combo
                    // boxes with wxSYS_COLOUR_BTNFACE. uxtheme cannot override this, so
                    // the dark background has to be returned from the control's parent.
                    const HWND control = reinterpret_cast<HWND>(lParam);
                    if (control && !::IsWindowEnabled(control) &&
                        IsEditOrComboBoxHandle(control))
                    {
                        HDC dc = reinterpret_cast<HDC>(wParam);
                        ::SetTextColor(dc, RGB(cbTheme::MutedText().Red(),
                                              cbTheme::MutedText().Green(),
                                              cbTheme::MutedText().Blue()));
                        ::SetBkColor(dc, RGB(cbTheme::Panel().Red(),
                                            cbTheme::Panel().Green(),
                                            cbTheme::Panel().Blue()));
                        return reinterpret_cast<LRESULT>(GetDarkControlBackgroundBrush());
                    }
                }
                break;

            case WM_NOTIFY:
                {
                    LPNMHDR notification = reinterpret_cast<LPNMHDR>(lParam);
                    if (notification && notification->code == static_cast<UINT>(NM_CUSTOMDRAW) &&
                        IsNativeClass(notification->hwndFrom, L"SysHeader32"))
                    {
                        return HandleDarkHeaderCustomDraw(notification);
                    }
                    if (notification && notification->code == static_cast<UINT>(NM_CUSTOMDRAW) &&
                        IsNativeClass(notification->hwndFrom, L"msctls_trackbar32"))
                    {
                        return HandleDarkTrackBarCustomDraw(notification);
                    }
                }
                break;

            case WM_ERASEBKGND:
                if (IsEtchedStaticLine(hwnd))
                    return 1;
                break;

            case WM_PAINT:
                if (IsEtchedStaticLine(hwnd))
                {
                    PAINTSTRUCT paint = {};
                    HDC dc = ::BeginPaint(hwnd, &paint);
                    RECT client = {};
                    ::GetClientRect(hwnd, &client);
                    FillRectWithColour(dc, client, cbTheme::Border());
                    ::EndPaint(hwnd, &paint);
                    return 0;
                }
                break;

            case WM_PRINT:
                if (IsEtchedStaticLine(hwnd))
                {
                    // Window DC, so the whole 2px sunken border can be painted.
                    RECT windowRect = {};
                    ::GetWindowRect(hwnd, &windowRect);
                    RECT paintRect = { 0, 0, windowRect.right - windowRect.left,
                                       windowRect.bottom - windowRect.top };
                    FillRectWithColour(reinterpret_cast<HDC>(wParam), paintRect, cbTheme::Border());
                    return 0;
                }
                break;

            case WM_PRINTCLIENT:
                if (IsEtchedStaticLine(hwnd))
                {
                    RECT client = {};
                    ::GetClientRect(hwnd, &client);
                    FillRectWithColour(reinterpret_cast<HDC>(wParam), client, cbTheme::Border());
                    return 0;
                }
                break;

            case WM_NCPAINT:
                if (IsEtchedStaticLine(hwnd))
                {
                    // wxStaticLine uses SS_GRAYRECT | SS_SUNKEN. For a 2px-high line the
                    // whole control area is its sunken non-client border and the client
                    // rectangle is empty, so WM_PAINT alone can never make it dark.
                    HDC dc = ::GetWindowDC(hwnd);
                    if (dc)
                    {
                        RECT windowRect = {};
                        ::GetWindowRect(hwnd, &windowRect);
                        RECT paintRect = { 0, 0, windowRect.right - windowRect.left,
                                           windowRect.bottom - windowRect.top };
                        FillRectWithColour(dc, paintRect, cbTheme::Border());
                        ::ReleaseDC(hwnd, dc);
                    }
                    return 0;
                }
                if (!::IsWindowEnabled(hwnd) && IsEditOrComboBoxHandle(hwnd))
                {
                    PaintDarkControlFrame(hwnd);
                    return 0;
                }
                break;

            default:
                break;
        }
    }

    return ::DefSubclassProc(hwnd, msg, wParam, lParam);
}

void InstallDarkThemeSubclass(HWND hwnd)
{
    if (!hwnd)
        return;

    static const wchar_t* const propertyName = L"cbDarkThemeSubclass";
    if (::GetPropW(hwnd, propertyName))
        return;

    if (::SetWindowSubclass(hwnd, DarkThemeSubclassProc, 1, 0))
        ::SetPropW(hwnd, propertyName, reinterpret_cast<HANDLE>(1));
}

BOOL CALLBACK ApplyNativeThemeToChildProc(HWND child, LPARAM lParam)
{
    const bool dark = lParam != 0;

    InstallDarkThemeSubclass(child);

    if (IsNativeClass(child, L"SysHeader32") ||
        IsNativeClass(child, L"ScrollBar") ||
        IsNativeClass(child, L"msctls_statusbar32") ||
        IsNativeClass(child, L"msctls_progress32") ||
        IsNativeClass(child, L"msctls_trackbar32"))
    {
        ApplyNativeThemeToHandle(child, GetDarkNativeThemeName(wxString()), dark);
        ::RedrawWindow(child, nullptr, nullptr,
                       RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
    }

    return TRUE;
}

void ApplyNativeThemeToChildren(HWND parent, bool dark)
{
    if (!parent)
        return;

    ::EnumChildWindows(parent, ApplyNativeThemeToChildProc, dark ? 1 : 0);
}

bool ApplyPreferredAppMode(int mode)
{
    const SetPreferredAppModeFunction setPreferredAppMode = GetSetPreferredAppModeFunction();
    if (!setPreferredAppMode)
        return false;

    const int previous = setPreferredAppMode(mode);
    (void)previous;

    const FlushMenuThemesFunction flushMenuThemes = GetFlushMenuThemesFunction();
    if (flushMenuThemes)
        flushMenuThemes();

    return true;
}
} // anonymous namespace

void cbApplyNativeAppTheme(bool dark)
{
    const bool available = IsWindowsDarkModeAvailable();
    static int lastMode = -1;
    const int mode = dark ? 2 : 3;

    if (!available)
        return;

    // PreferredAppMode: 0 = default, 1 = allow dark, 2 = force dark, 3 = force light.
    // ForceDark/ForceLight make native menus, title bars and common controls use their dark
    // variants even when the Windows system theme itself is not dark.
    if (mode == lastMode)
        return;

    if (ApplyPreferredAppMode(mode))
        lastMode = mode;
}

void cbApplyNativeTheme(wxWindow* window, bool dark)
{
    if (!window)
        return;

    const HWND hwnd = static_cast<HWND>(window->GetHWND());
    if (!hwnd)
        return;

    const wxClassInfo* themeClassInfo = window->GetClassInfo();
    const wxString themeClassName = themeClassInfo ? themeClassInfo->GetClassName() : wxString();

    if (window->IsTopLevel())
    {
        // DWMWA_USE_IMMERSIVE_DARK_MODE is 20 on Windows 10 2004+ and 19 on older builds.
        // Load the function dynamically so that the application still starts on systems
        // without dwmapi.dll (or with an older version of it).
        const DwmSetWindowAttributeFunction setDwmAttribute = GetDwmSetWindowAttributeFunction();
        if (setDwmAttribute)
        {
            const BOOL enable = dark ? TRUE : FALSE;
            HRESULT result = setDwmAttribute(hwnd, 20, &enable, sizeof(enable));
            if (FAILED(result))
                result = setDwmAttribute(hwnd, 19, &enable, sizeof(enable));
            (void)result;
        }
    }

    if (IsWindowsDarkModeAvailable())
    {
        const AllowDarkModeForWindowFunction allowDarkMode = GetAllowDarkModeForWindowFunction();
        if (allowDarkMode)
        {
            const BOOL allowed = allowDarkMode(hwnd, dark ? TRUE : FALSE);
            (void)allowed;
        }
    }

    // SetWindowTheme(..., L"DarkMode_DarkTheme"/L"DarkMode_Explorer", ...) switches native
    // common controls (edits, lists, trees, buttons, tabs, scrollbars) to their dark
    // variants. wxWidgets 3.2 does not do this automatically, so apply it to every window,
    // including top-level frames/dialogs and generic wxWindowNR containers. On Windows 11
    // 24H2+ this also switches the non-client scroll bars of scrolled windows.
    const wxString className = themeClassName;
    ApplyNativeThemeToHandle(hwnd, GetDarkNativeThemeName(className), dark);

    if (className.Contains("ComboBox") || className.Contains("Choice"))
        ApplyNativeThemeToComboBoxInternals(hwnd, dark);

    // Native children which are not wxWindow objects (list headers, status bars and the
    // scroll bars created by common controls) need to be themed separately.
    ApplyNativeThemeToChildren(hwnd, dark);
    InstallDarkThemeSubclass(hwnd);
}

namespace
{
// Undocumented messages used by Windows (UAH) to draw the menu bar. wxWidgets 3.3
// handles them in its own dark mode implementation; wx 3.2 does not, so the menu
// bar would remain light even when the application dark mode is active.
const unsigned int CB_MENUBAR_DRAWMENU = 0x0091;
const unsigned int CB_MENUBAR_DRAWMENUITEM = 0x0092;

struct CbMenuBarDrawMenu
{
    HMENU hmenu;
    HDC hdc;
    DWORD dwReserved;
};

struct CbMenuBarMenuItem
{
    int iPosition;
};

struct CbMenuBarDrawMenuItem
{
    DRAWITEMSTRUCT dis;
    CbMenuBarDrawMenu mbdm;
    CbMenuBarMenuItem mbmi;
};

void DrawMenuBarItemText(HDC dc, RECT& rect, const wchar_t* text,
                         const wxColour& colour, UINT flags)
{
    NONCLIENTMETRICSW metrics = {};
    metrics.cbSize = sizeof(metrics);

    HFONT font = nullptr;
    if (::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0))
        font = ::CreateFontIndirectW(&metrics.lfMenuFont);

    const HGDIOBJ oldFont = font ? ::SelectObject(dc, font) : nullptr;
    const int oldMode = ::SetBkMode(dc, TRANSPARENT);
    const COLORREF oldColour = ::SetTextColor(dc, RGB(colour.Red(), colour.Green(), colour.Blue()));

    ::DrawTextW(dc, text, -1, &rect, flags);

    ::SetTextColor(dc, oldColour);
    ::SetBkMode(dc, oldMode);
    if (oldFont)
        ::SelectObject(dc, oldFont);
    if (font)
        ::DeleteObject(font);
}
} // anonymous namespace

bool cbHandleDarkMenuBarMessage(wxWindow* window, unsigned int msg,
                                wxUIntPtr wParam, wxIntPtr lParam)
{
    // The menu bar itself has no wxWindow MSWProc, the messages are sent to the frame.
    // wParam is unused for the messages handled here.
    wxUnusedVar(wParam);

    if (!window || !lParam)
        return false;

    const HWND hwnd = static_cast<HWND>(window->GetHWND());
    if (!hwnd)
        return false;

    switch (msg)
    {
        case CB_MENUBAR_DRAWMENU:
        {
            const CbMenuBarDrawMenu* const drawMenu =
                reinterpret_cast<const CbMenuBarDrawMenu*>(lParam);
            if (!drawMenu || !drawMenu->hdc)
                return false;

            MENUBARINFO menuInfo = {};
            menuInfo.cbSize = sizeof(menuInfo);
            if (!::GetMenuBarInfo(hwnd, OBJID_MENU, 0, &menuInfo))
                return false;

            RECT windowRect = {};
            ::GetWindowRect(hwnd, &windowRect);
            ::OffsetRect(&menuInfo.rcBar, -windowRect.left, -windowRect.top);

            FillRectWithColour(drawMenu->hdc, menuInfo.rcBar, cbTheme::Window());
            return true;
        }

        case CB_MENUBAR_DRAWMENUITEM:
        {
            const CbMenuBarDrawMenuItem* const drawItem =
                reinterpret_cast<const CbMenuBarDrawMenuItem*>(lParam);
            if (!drawItem || drawItem->dis.CtlType != ODT_MENU || !drawItem->dis.hDC)
                return false;

            wchar_t text[256] = {};
            MENUITEMINFOW itemInfo = {};
            itemInfo.cbSize = sizeof(itemInfo);
            itemInfo.fMask = MIIM_STRING;
            itemInfo.dwTypeData = text;
            itemInfo.cch = WXSIZEOF(text);

            // DRAWITEMSTRUCT::itemID is not valid for the menu bar, the undocumented
            // MenuBarMenuItem::iPosition field is the reliable item index.
            if (!::GetMenuItemInfoW(reinterpret_cast<HMENU>(drawItem->dis.hwndItem),
                                    drawItem->mbmi.iPosition, TRUE, &itemInfo))
                return false;

            const UINT state = drawItem->dis.itemState;
            const bool hot = (state & (ODS_HOTLIGHT | ODS_SELECTED)) != 0;
            const bool disabled = (state & (ODS_GRAYED | ODS_DISABLED | ODS_INACTIVE)) != 0;

            FillRectWithColour(drawItem->dis.hDC, drawItem->dis.rcItem,
                               hot ? cbTheme::Selection() : cbTheme::Window());

            RECT textRect = drawItem->dis.rcItem;
            UINT flags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
            if (state & ODS_NOACCEL)
                flags |= DT_HIDEPREFIX;

            DrawMenuBarItemText(drawItem->dis.hDC, textRect, text,
                                disabled ? cbTheme::MutedText() : cbTheme::Text(), flags);
            return true;
        }

        default:
            break;
    }

    return false;
}

void cbFixDarkMenuBarBottomLine(wxWindow* window)
{
    if (!window)
        return;

    const HWND hwnd = static_cast<HWND>(window->GetHWND());
    if (!hwnd || !::GetMenu(hwnd))
        return;

    const HDC dc = ::GetWindowDC(hwnd);
    if (!dc)
        return;

    RECT windowRect = {};
    ::GetWindowRect(hwnd, &windowRect);

    RECT rc = {};
    ::GetClientRect(hwnd, &rc);
    ::MapWindowPoints(hwnd, HWND_DESKTOP, reinterpret_cast<LPPOINT>(&rc), 2);
    ::OffsetRect(&rc, -windowRect.left, -windowRect.top);

    // Drawing the menu bar in WM_MENUBAR_DRAWMENU leaves the last pixel row above
    // the client area unpainted, so paint it using the menu bar colour.
    rc.bottom = rc.top;
    rc.top--;

    FillRectWithColour(dc, rc, cbTheme::Window());
    ::ReleaseDC(hwnd, dc);
}
#else
void cbApplyNativeTheme(cb_unused wxWindow* window, cb_unused bool dark)
{
}

void cbApplyNativeAppTheme(cb_unused bool dark)
{
}
#endif

namespace
{
wxString GetThemeWindowClassName(const wxWindow* window)
{
    if (!window)
        return wxString();

    const wxClassInfo* classInfo = window->GetClassInfo();
    return classInfo ? wxString(classInfo->GetClassName()) : wxString();
}

bool IsTextOnlyThemeControl(const wxString& className)
{
    // These controls draw their label directly on the parent background, so they only
    // need a matching text colour (a background colour would create visible boxes).
    return className.Contains("StaticText")
        || className.Contains("CheckBox")
        || className.Contains("RadioButton")
        || className.Contains("RadioBox")
        || className.Contains("StaticLine");
}

bool IsDataThemeControl(const wxString& className)
{
    static const wxString tokens[] =
    {
        "TextCtrl", "ListCtrl", "TreeCtrl", "ComboBox", "Choice", "SpinCtrl",
        "SearchCtrl", "HeaderCtrl", "ListBox", "DataView", "Grid", "PropertyGrid"
    };

    for (const wxString& token : tokens)
    {
        if (className.Contains(token))
            return true;
    }
    return false;
}

bool IsNativeThemeControl(const wxString& className)
{
    static const wxString tokens[] =
    {
        "TextCtrl", "Button", "ComboBox", "Choice", "SpinCtrl", "SpinButton",
        "CheckBox", "RadioButton", "RadioBox", "ScrollBar", "Slider", "Gauge",
        "ToolBar", "MenuBar", "StatusBar", "Scintilla", "StyledTextCtrl", "Picker",
        "StaticText", "StaticLine", "Hyperlink", "SearchCtrl", "CollapsiblePane",
        "TaskBarIcon", "ListCtrl", "TreeCtrl", "HeaderCtrl", "ListBox", "Notebook",
        "TabCtrl", "DataView", "Grid", "PropertyGrid"
    };

    for (const wxString& token : tokens)
    {
        if (className.Contains(token))
            return true;
    }
    return false;
}

bool IsLoggerPage(const wxWindow* window, const wxString& className)
{
    // Log controls are direct pages of the "Logs & others" wxAuiNotebook. Their colours are
    // owned by TextCtrlLogger/ListCtrlLogger (the logs use a slightly darker background than
    // the side panels), so don't override them here.
    if (!className.Contains("TextCtrl") && !className.Contains("ListCtrl"))
        return false;

    const wxWindow* parent = window ? window->GetParent() : nullptr;
    return parent && GetThemeWindowClassName(parent).Contains("AuiNotebook");
}

#ifdef __WXMSW__
void RestoreNativeDataControlColours(HWND hwnd)
{
    if (!hwnd)
        return;

    wchar_t className[64] = {};
    if (!::GetClassNameW(hwnd, className, WXSIZEOF(className)))
        return;

    // wxWidgets implements SetBackgroundColour() for the native tree and list controls by
    // sending TVM_SETBKCOLOR/LVM_SETBKCOLOR. Setting wxNullColour does not send the inverse
    // message, so without this the black colour of the dark theme survives the switch back
    // to the light theme. Restore the system colours explicitly.
    const COLORREF background = ::GetSysColor(COLOR_WINDOW);
    const COLORREF text = ::GetSysColor(COLOR_WINDOWTEXT);

    if (wxString(className).IsSameAs("SysTreeView32", false))
    {
        ::SendMessageW(hwnd, TVM_SETBKCOLOR, 0, static_cast<LPARAM>(background));
        ::SendMessageW(hwnd, TVM_SETTEXTCOLOR, 0, static_cast<LPARAM>(text));
    }
    else if (wxString(className).IsSameAs("SysListView32", false))
    {
        ::SendMessageW(hwnd, LVM_SETBKCOLOR, 0, static_cast<LPARAM>(background));
        ::SendMessageW(hwnd, LVM_SETTEXTCOLOR, 0, static_cast<LPARAM>(text));
        ::SendMessageW(hwnd, LVM_SETTEXTBKCOLOR, 0, static_cast<LPARAM>(background));
    }
    else
        return;

    ::RedrawWindow(hwnd, nullptr, nullptr,
                   RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
}
#endif // __WXMSW__
} // anonymous namespace

void cbApplyAppThemeToWindow(wxWindow* window, bool dark)
{
    if (!window)
        return;

    // Freeze the top-level window for the whole recursive pass. Child windows no longer
    // call Refresh() individually; one RedrawWindow() at the end repaints the complete tree.
    const bool topLevel = window->IsTopLevel();
    if (topLevel)
        window->Freeze();

    // Apply the native dark bits even to controls which keep their system-drawn appearance.
    cbApplyNativeTheme(window, dark);

    const wxString className = GetThemeWindowClassName(window);
    const bool textOnly = IsTextOnlyThemeControl(className);
    const bool toolbar = className.Contains("ToolBar");
    const bool auiNotebook = className.Contains("AuiNotebook");
    const bool auiTabCtrl = className.Contains("AuiTabCtrl");
    const bool loggerPage = IsLoggerPage(window, className);
    const bool dataControl = IsDataThemeControl(className);
    const bool scintilla = className.Contains("Scintilla") || className.Contains("StyledTextCtrl");
    const bool menuOrStatus = className.Contains("MenuBar") || className.Contains("StatusBar");

    if (topLevel)
    {
        window->SetBackgroundColour(dark ? cbTheme::Window() : wxNullColour);
        window->SetForegroundColour(dark ? cbTheme::Text() : wxNullColour);
    }
    else if (textOnly)
    {
        window->SetForegroundColour(dark ? cbTheme::Text() : wxNullColour);
    }
    else if (toolbar)
    {
        window->SetBackgroundColour(dark ? cbTheme::Window() : wxNullColour);
        window->SetForegroundColour(dark ? cbTheme::Text() : wxNullColour);
    }
    else if (auiNotebook || auiTabCtrl)
    {
        // The tab strip is deliberately darker than the pages; the active tab is painted
        // with the editor colour by the dark tab art.
        window->SetBackgroundColour(dark ? cbTheme::Window() : wxNullColour);
        window->SetForegroundColour(dark ? cbTheme::Text() : wxNullColour);
    }
    else if (loggerPage)
    {
        // Colours are applied by the loggers.
    }
    else if (dataControl)
    {
        if (dark)
        {
            window->SetBackgroundColour(cbTheme::Panel());
            window->SetForegroundColour(cbTheme::Text());
        }
        else
        {
            // Use explicit light system colours instead of wxNullColour. Native controls
            // remember the background which was stored in the control itself (TVM_SETBKCOLOR,
            // LVM_SETBKCOLOR) and do not necessarily reset it when the wx colour is cleared.
            window->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
            window->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
#ifdef __WXMSW__
            RestoreNativeDataControlColours(static_cast<HWND>(window->GetHWND()));
#endif
        }
    }
    else if (scintilla)
    {
        window->SetBackgroundColour(dark ? cbTheme::Panel() : wxNullColour);
        window->SetForegroundColour(dark ? cbTheme::EditorText() : wxNullColour);
    }
    else if (menuOrStatus)
    {
        window->SetBackgroundColour(dark ? cbTheme::Window() : wxNullColour);
        window->SetForegroundColour(dark ? cbTheme::Text() : wxNullColour);
    }
    else if (!IsNativeThemeControl(className))
    {
        // Panels, splitters, generic notebooks, AUI containers, HTML windows, etc.
        window->SetBackgroundColour(dark ? cbTheme::Panel() : wxNullColour);
        window->SetForegroundColour(dark ? cbTheme::Text() : wxNullColour);
    }
    else
    {
        // Native buttons and other controls which can't be fully recoloured: at least make
        // their text readable. The dark uxtheme variant is enabled above.
        window->SetBackgroundColour(dark ? cbTheme::Panel() : wxNullColour);
        window->SetForegroundColour(dark ? cbTheme::Text() : wxNullColour);
    }

    for (wxWindowList::compatibility_iterator node = window->GetChildren().GetFirst();
         node; node = node->GetNext())
    {
        cbApplyAppThemeToWindow(static_cast<wxWindow*>(node->GetData()), dark);
    }

    if (topLevel)
    {
        window->Thaw();

        if (window->GetHWND())
        {
            // Force the complete window tree, including native children, to repaint with the
            // new theme. This is the only repaint performed for this top-level window; the
            // individual controls did not request any redraw while the theme was applied.
            ::RedrawWindow(static_cast<HWND>(window->GetHWND()), nullptr, nullptr,
                           RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_UPDATENOW);
        }
    }
}

void cbApplyThemeToAuiNotebooks(wxWindow* root, bool dark)
{
    if (!root)
        return;

    if (cbAuiNotebook* notebook = dynamic_cast<cbAuiNotebook*>(root))
    {
        // wxAuiNotebook has its own wxAuiManager which paints the client area behind the
        // pages using a dock art background. The default art uses the light system face
        // colour, so theme it explicitly. The pages (editors, logs, trees) paint their own
        // backgrounds on top of it; the tab strip is painted by the dark tab art.
        if (wxAuiManager* manager = wxAuiManager::GetManager(notebook))
        {
            if (wxAuiDockArt* art = manager->GetArtProvider())
            {
                wxAuiDefaultDockArt defaultArt;
                art->SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR,
                               dark ? cbTheme::Panel()
                                    : defaultArt.GetColour(wxAUI_DOCKART_BACKGROUND_COLOUR));
                art->SetColour(wxAUI_DOCKART_SASH_COLOUR,
                               dark ? cbTheme::Border()
                                    : defaultArt.GetColour(wxAUI_DOCKART_SASH_COLOUR));
                art->SetColour(wxAUI_DOCKART_BORDER_COLOUR,
                               dark ? cbTheme::Border()
                                    : defaultArt.GetColour(wxAUI_DOCKART_BORDER_COLOUR));
            }
        }
    }

    for (wxWindowList::compatibility_iterator node = root->GetChildren().GetFirst();
         node; node = node->GetNext())
    {
        cbApplyThemeToAuiNotebooks(static_cast<wxWindow*>(node->GetData()), dark);
    }
}

// this doesn't work under wxGTK, and is only needed on wxMSW, we work around it on wxGTK
#ifdef __WXMSW__
void SetSettingsIconsStyle(wxListCtrl* lc, SettingsIconsStyle style)
{
    long flags = lc->GetWindowStyleFlag();
    switch (style)
    {
        case sisNoIcons: flags = (flags & ~wxLC_MASK_TYPE) | wxLC_LIST; break;
        default: flags = (flags & ~wxLC_MASK_TYPE) | wxLC_ICON; break;
    }
    lc->SetWindowStyleFlag(flags);
}
#else
void SetSettingsIconsStyle(cb_unused wxListCtrl* lc, cb_unused SettingsIconsStyle style) {}
#endif

SettingsIconsStyle GetSettingsIconsStyle(cb_unused wxListCtrl* lc)
{
    return GetSettingsIconsStyle();
}

SettingsIconsStyle GetSettingsIconsStyle()
{
    return SettingsIconsStyle(Manager::Get()->GetConfigManager(_T("app"))->ReadInt(_T("/environment/settings_size"), 0));
}

wxRect cbGetMonitorRectForWindow(wxWindow *window)
{
    wxRect monitorRect;
    if (wxDisplay::GetCount() > 0)
    {
        int displayIdx = wxDisplay::GetFromWindow(window);
        if (displayIdx == wxNOT_FOUND)
            displayIdx = 0;
        wxDisplay display(displayIdx);
        monitorRect = display.GetClientArea();
        // This is needed because on Linux the client area returned for the first monitor in a twin
        // monitor setup with nVidia card is spanning the two monitors.
        // The intersection function will return just the client for the specified monitor.
        monitorRect = display.GetGeometry().Intersect(monitorRect);
    }
    else
    {
        int width, height;
        wxDisplaySize(&width, &height);
        monitorRect = wxRect(0, 0, width, height);
    }
    return monitorRect;
}

cbChildWindowPlacement cbGetChildWindowPlacement(ConfigManager &appConfig)
{
    int intChildWindowPlacement = appConfig.ReadInt(wxT("/dialog_placement/child_placement"),
                                                    int(cbChildWindowPlacement::CenterOnParent));
    if (intChildWindowPlacement < 0 || intChildWindowPlacement >= 3)
        intChildWindowPlacement = 0;

    return cbChildWindowPlacement(intChildWindowPlacement);
}

void PlaceWindow(wxTopLevelWindow *w, cbPlaceDialogMode mode, bool enforce)
{
    if (!w)
        cbThrow(_T("Passed nullptr pointer to PlaceWindow."));

    int the_mode = int(mode);

    if (!enforce)
    {
        ConfigManager *cfg = Manager::Get()->GetConfigManager(_T("app"));
        const cbChildWindowPlacement placement =  cbGetChildWindowPlacement(*cfg);
        switch (placement)
        {
            case cbChildWindowPlacement::CenterOnParent:
                w->CenterOnParent();
                return;
            case cbChildWindowPlacement::CenterOnDisplay:
            {
                if (mode == pdlBest)
                    the_mode = cfg->ReadInt(_T("/dialog_placement/dialog_position"), (int) pdlCentre);
                else
                    the_mode = (int) mode;
                break;
            }
            case cbChildWindowPlacement::LeaveToWM:
                return;
        }
    }

    wxWindow* referenceWindow = Manager::Get()->GetAppWindow();
    if (!referenceWindow)    // no application window available, so this is as good as we can get
        referenceWindow = w;

    const wxRect monitorRect = cbGetMonitorRectForWindow(referenceWindow);
    wxRect windowRect = w->GetRect();

    switch(the_mode)
    {
        case pdlCentre:
        {
            windowRect.x = monitorRect.x + (monitorRect.width  - windowRect.width)/2;
            windowRect.y = monitorRect.y  + (monitorRect.height - windowRect.height)/2;
        }
        break;


        case pdlHead:
        {
            windowRect.x = monitorRect.x + (monitorRect.width  - windowRect.width)/2;
            windowRect.y = monitorRect.y  + (monitorRect.height - windowRect.height)/3;
        }
        break;


        case pdlConstrain:
        {
            int x1 = windowRect.x;
            int x2 = windowRect.x + windowRect.width;
            int y1 = windowRect.y;
            int y2 = windowRect.y + windowRect.height;

            if (windowRect.width > monitorRect.width) // cannot place without clipping, so centre it
            {
                x1 = monitorRect.x + (monitorRect.width  - windowRect.width)/2;
                x2 = x1 + windowRect.width;
            }
            else
            {
                x2 = std::min(monitorRect.GetRight(), windowRect.GetRight());
                x1 = std::max(x2 - windowRect.width, monitorRect.x);
                x2 = x1 + windowRect.width;
            }
            if (windowRect.height > monitorRect.height) // cannot place without clipping, so centre it
            {
                y1 = monitorRect.y + (monitorRect.height  - windowRect.height)/2;
                y2 = y1 + windowRect.height;
            }
            else
            {
                y2 = std::min(monitorRect.GetBottom(), windowRect.GetBottom());
                y1 = std::max(y2 - windowRect.height, monitorRect.y);
                y2 = y1 + windowRect.height;
            }
            windowRect = wxRect(x1, y1, x2-x1, y2-y1);
        }
        break;

        case pdlClip:
        {
            int x1 = windowRect.x;
            int x2 = windowRect.x + windowRect.width;
            int y1 = windowRect.y;
            int y2 = windowRect.y + windowRect.height;

            x1 = std::max(x1, monitorRect.x);
            x2 = std::min(x2, monitorRect.GetRight());
            y1 = std::max(y1, monitorRect.y);
            y2 = std::min(y2, monitorRect.GetBottom());

            windowRect = wxRect(x1, y1, x2-x1, y2-y1);
        }
        break;
    }

    w->SetSize(windowRect.x,  windowRect.y, windowRect.width, windowRect.height, wxSIZE_ALLOW_MINUS_ONE);
}

void cbFixWindowSizeAndPlace(wxTopLevelWindow* const w)
{
    if (w == nullptr)
        return;

    const int displayNumber = wxDisplay::GetFromWindow(w); // The display number this window is targeted from the layout file
    if (displayNumber == wxNOT_FOUND)
    {
        // this window is not on a valid display.
        // Place the window to the centre of the current display
        const wxDisplay currentDisplay;

        const int displayHeight = currentDisplay.GetClientArea().GetHeight();
        const int displayWidth  = currentDisplay.GetClientArea().GetWidth();
        const int panelHeight   = w->GetSize().GetHeight();
        const int panelWidth    = w->GetSize().GetWidth();
        if (panelHeight > displayHeight ||
            panelWidth  > displayWidth)
        {
            // If the window is bigger then the current display
            // Rescale the window to 1/3 of the current resolution
            // try to keep the aspect ratio
            const float windowRatio = (float) panelHeight / panelWidth;
            const float scaledWidth = displayWidth / 3.0f;
            float scaledHeight = scaledWidth * windowRatio;
            if (scaledHeight > displayHeight)
            {
                // If the window is still to tall we break the aspect ratio
                scaledHeight = displayHeight / 3.0f;
            }
            w->SetSize(scaledWidth, scaledHeight);
        }
        // Replace it to the centre of the screen...
        Manager::Get()->GetLogManager()->Log(wxString::Format(_("Window \"%s\" was on an invalid display, relocate it to main display"), w->GetTitle().ToUTF8().data()));
        PlaceWindow(w, pdlCentre, true);
    }
}

DirAccessCheck cbDirAccessCheck(const wxString& dir)
{
    wxString actualDir = dir;
    // append ending path separator if needed
    if (actualDir.Last() != _T('/') && actualDir.Last() != _T('\\'))
        actualDir << wxFILE_SEP_PATH;

    if (!wxDirExists(actualDir))
        return dacInvalidDir;

    wxString testFile = wxFileName::CreateTempFileName(actualDir);
    if (!testFile.IsEmpty())
    {
        // ok, write-access confirmed
        // now remove the temporary file and return success
        wxRemoveFile(testFile);
        return dacReadWrite;
    }

    // if we reached here, the directory is not writable
    return dacReadOnly;
}


namespace platform
{
    windows_version_t cb_get_os()
    {
        if (!platform::windows)
            return winver_NotWindows;

        windows_version_t version = winver_UnknownWindows;

        int Major = 0;
        int Minor = 0;
        int Micro = 0;
#if wxCHECK_VERSION(3, 1, 1)
        switch (wxGetOsVersion(&Major, &Minor, &Micro))
#else
        switch (wxGetOsVersion(&Major, &Minor))
#endif
        {
            case wxOS_WINDOWS_9X:
                version = winver_Windows9598ME;
            break;
            case wxOS_WINDOWS_NT:
                switch (Major)
                {
                case 5:
                    if (Minor == 0)
                        version = winver_WindowsNT2000;
                    else if (Minor == 1)
                        version = winver_WindowsXP;
                    else if (Minor == 2)
                        version = winver_WindowsServer2003;

                break;
                case 6:
                    if (Minor == 0)
                        version = winver_WindowsVista;
                    else if (Minor == 1)
                        version = winver_Windows7;
                    else if ((Minor == 2) || (Minor == 3))
                        version = winver_Windows8;

                break;
                case 10:
                    if (Minor == 0)
                        version = (Micro < 22000) ? winver_Windows10 : winver_Windows11;
                }

            break;
            default:
                ;
        }

        return version;
    }

    windows_version_t WindowsVersion()
    {
        static const windows_version_t theOS = cb_get_os();
        return theOS;
    }
}

// returns the real path of a file by resolving symlinks
// not yet optimal but should do for now
// one thing that's not checked yet are circular symlinks - watch out!
wxString realpath(const wxString& path)
{
#ifdef __WXMSW__
    // no symlinks support on windows
    return path;
#else
    char buf[2048] = {};
    struct stat buffer;
    std::string ret = (const char*)cbU2C(path);
    size_t lastPos = 0;
    size_t slashPos = ret.find('/', lastPos);
    while (slashPos != std::string::npos)
    {
        if (lstat(ret.substr(0, slashPos).c_str(), &buffer) == 0)
        {
            if (S_ISLNK(buffer.st_mode))
            {
                int s = readlink(ret.substr(0, slashPos).c_str(), buf, sizeof(buf));
                buf[s] = 0;
                if (s > 0 && buf[0] != '/' && buf[0] != '~')
                {
                    // relative
                    ret = ret.substr(0, lastPos) + buf + ret.substr(slashPos, ret.size() - slashPos);
                }
                else
                {
                    // absolute
                    ret = buf + ret.substr(slashPos, ret.size() - slashPos);

                    // start again at the beginning in case the path returned also
                    // has symlinks. For example if using /etc/alternatives this will
                    // be the case
                    s = 0;
                }
                slashPos = s;
            }
        }

        while (ret[++slashPos] == '/')
            ;
        lastPos = slashPos;
        slashPos = ret.find('/', slashPos);
    }
    return cbC2U(ret.c_str());
#endif
}

int cbMessageBox(const wxString& message, const wxString& caption, int style, wxWindow *parent, int x, int y)
{
    if (!parent)
        parent = Manager::Get()->GetAppWindow();

    if (cbIsAppThemeDark())
    {
        // The native Windows message box always uses the light system colours and ignores
        // the dark app mode, which leaves a large white surface on screen. The generic
        // dialog is made of normal wxWidgets controls, so the dark palette can be applied
        // to it and its buttons just like to any other Code::Blocks dialog.
        wxGenericMessageDialog dlg(parent, message, caption, style, wxPoint(x, y));
        cbApplyAppThemeToWindow(&dlg, true);
        PlaceWindow(&dlg);
        return dlg.ShowModal();
    }

    // Cannot create a wxMessageDialog with a nullptr as parent
    if (!parent)
    {
      // wxMessage*Box* returns any of: wxYES, wxNO, wxCANCEL, wxOK.
      int answer = wxMessageBox(message, caption, style, parent, x, y);
      switch (answer)
      {
        // map answer to the one of wxMessage*Dialog* to ensure compatibility
        case (wxOK):
          return wxID_OK;
        case (wxCANCEL):
          return wxID_CANCEL;
        case (wxYES):
          return wxID_YES;
        case (wxNO):
          return wxID_NO;
        default:
          return -1; // NOTE: Cannot happen unless wxWidgets API changes
      }
    }

    wxMessageDialog dlg(parent, message, caption, style, wxPoint(x,y));
    PlaceWindow(&dlg);
    // wxMessage*Dialog* returns any of wxID_OK, wxID_CANCEL, wxID_YES, wxID_NO
    return dlg.ShowModal();
}

DLLIMPORT int cbGetSingleChoiceIndex(const wxString& message, const wxString& caption,
                                     const wxArrayString& choices, wxWindow *parent,
                                     const wxSize &size, int initialSelection)
{
    if (!parent)
        parent = Manager::Get()->GetAppWindow();

    wxSingleChoiceDialog dialog(parent, message, caption, choices);
    dialog.SetSelection(initialSelection);
    dialog.SetSize(size);
    PlaceWindow(&dialog);
    return (dialog.ShowModal() == wxID_OK ? dialog.GetSelection() : -1);
}

DLLIMPORT wxArrayInt cbGetMultiChoiceDialog(const wxString& message, const wxString& caption,
                                     const wxArrayString& choices, wxWindow *parent,
                                     const wxSize& size, const wxArrayInt& initialSelection)
{
    if (!parent)
        parent = Manager::Get()->GetAppWindow();

    wxMultiChoiceDialog dialog(parent, message, caption, choices);
    dialog.SetSelections(initialSelection);
    dialog.SetSize(size);
    PlaceWindow(&dialog);

    if (dialog.ShowModal() == wxID_OK)
        return dialog.GetSelections();
    else
        return wxArrayInt();
}

const char* cbGetTextFromUserPromptStr = wxGetTextFromUserPromptStr;

wxString cbGetTextFromUser(const wxString& message, const wxString& caption, const wxString& defaultValue,
                           wxWindow *parent, wxCoord x, wxCoord y, bool centre)
{
    if (!parent)
        parent = Manager::Get()->GetAppWindow();

    long style = wxTextEntryDialogStyle;
    if (centre)
        style |= wxCENTRE;
    else
        style &= ~wxCENTRE;

    wxTextEntryDialog dialog(parent, message, caption, defaultValue, style, wxPoint(x, y));
    PlaceWindow(&dialog);
    wxString str;
    if (dialog.ShowModal() == wxID_OK)
        str = dialog.GetValue();
    return str;
}


std::unique_ptr<wxImageList> cbProjectTreeImages::MakeImageList(int baseSize, wxWindow &treeParent)
{
    static const wxString imgs[fvsLast] =
    {
        // NOTE: Keep in sync with FileVisualState in globals.h!

        // The following are related to (editable, source-) file states
        "file",                  // fvsNormal
        "file-missing",          // fvsMissing,
        "file-modified",         // fvsModified,
        "file-readonly",         // fvsReadOnly,

        // The following are related to version control systems (vc)
        "rc-file-added",         // fvsVcAdded,
        "rc-file-conflict",      // fvsVcConflict,
        "rc-file-missing",       // fvsVcMissing,
        "rc-file-modified",      // fvsVcModified,
        "rc-file-outofdate",     // fvsVcOutOfDate,
        "rc-file-uptodate",      // fvsVcUpToDate,
        "rc-file-requireslock",  // fvsVcRequiresLock,
        "rc-file-external",      // fvsVcExternal,
        "rc-file-gotlock",       // fvsVcGotLock,
        "rc-file-lockstolen",    // fvsVcLockStolen,
        "rc-file-mismatch",      // fvsVcMismatch,
        "rc-file-noncontrolled", // fvsVcNonControlled,

        // The following are related to C::B workspace/project/folder/virtual
        "workspace",             // fvsWorkspace,         WorkspaceIconIndex()
        "workspace-readonly",    // fvsWorkspaceReadOnly, WorkspaceIconIndex(true)
        "project",               // fvsProject,           ProjectIconIndex()
        "project-readonly",      // fvsProjectReadOnly,   ProjectIconIndex(true)
        "folder_open",           // fvsFolder,            FolderIconIndex()
        "vfolder_open",          // fvsVirtualFolder,     VirtualFolderIconIndex()
    };

    const double scaleFactor = cbGetContentScaleFactor(treeParent);
    const int targetHeight = wxRound(baseSize * scaleFactor);
    const int size = cbFindMinSize16to64(targetHeight);

    int imageListSize;
    std::unique_ptr<wxImageList> images = cbMakeScaledImageList(size, scaleFactor, imageListSize);

    wxString prefix(ConfigManager::ReadDataPath() + "/resources.zip#zip:images/tree/");
#if wxCHECK_VERSION(3, 1, 6)
    prefix << "svg/";
    for (const wxString &img : imgs)
    {
        wxBitmap bmp = cbLoadBitmapBundleFromSVG(prefix + img + ".svg", wxSize(baseSize, baseSize)).GetBitmap(wxSize(imageListSize, imageListSize));
        cbAddBitmapToImageList(*images, bmp, imageListSize, imageListSize, scaleFactor);
    }
#else
    prefix << wxString::Format("%dx%d/", imageListSize, imageListSize);
    for (const wxString &img : imgs)
    {
        wxBitmap bmp = cbLoadBitmap(prefix + img + ".png");
        cbAddBitmapToImageList(*images, bmp, imageListSize, imageListSize, scaleFactor);
    }
#endif

    return images;
}

int cbProjectTreeImages::WorkspaceIconIndex(bool read_only)
{
    if (read_only)
        return (int)fvsWorkspaceReadOnly;

    return (int)fvsWorkspace;
}

int cbProjectTreeImages::ProjectIconIndex(bool read_only)
{
    if (read_only)
        return (int)fvsProjectReadOnly;

    return (int)fvsProject;
}

int cbProjectTreeImages::FolderIconIndex()
{
    return (int)fvsFolder;
}

int cbProjectTreeImages::VirtualFolderIconIndex()
{
    return (int)fvsVirtualFolder;
}
