// FilterDialog.cpp: 实现文件
//
#include <winsock2.h>
#include "FilterDialog.h"
#include "DockingFeature/resource.h"
#include "ThemeRenderer.h"
#include <windows.h>    // 包含Windows API头文件
#include <commctrl.h>   // 包含ListCtrl API宏定义（如ListView_InsertItem等）
#include "FilterResource.h"
#include "CreateRuleDialog.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <regex>
#include "NppInterface.h"

extern CreateRuleDialog createRuleDlg;
extern NppData nppData;

// FilterDialog 对话框
FilterDialog::FilterDialog()
    : DockingDlgInterface(IDD_FILTER_DLG1)
    , _bStartupFinish(FALSE)
    , _hListCtrl(nullptr)
    , _hDefaultListProc(nullptr)
{

}

FilterDialog::~FilterDialog()
{
}


void FilterDialog::init(HINSTANCE hInst, HWND hParent)
{
    DockingDlgInterface::init(hInst, hParent);
}

ToolBarButtonUnit toolBarIcons[] = {
    {IDM_EX_ADD,           IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_FILTER_ADD,        0},
    {IDM_EX_DEL,           IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_FILTER_DEL,        0},
    {IDM_EX_SAVEFILTER,    IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_FILTER_SAVE,       0},
    {IDM_EX_IMPORTFILTER,  IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_FILTER_IMPORT,     0},
    {IDM_EX_REFRESH,       IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDI_SEPARATOR_ICON, IDB_FILTER_REFRESH,    0},
};

void FilterDialog::InitialDialog()
{
    /* get handle of dialogs */
    _hListCtrl = ::GetDlgItem(_hSelf, IDC_FILTER_LIST);

    /* subclass tree */
    ::SetWindowLongPtr(_hListCtrl, GWLP_USERDATA, (LONG_PTR)this);
    _hDefaultListProc = (WNDPROC)::SetWindowLongPtr(_hListCtrl, GWLP_WNDPROC, (LONG_PTR)wndDefaultListProc);
    
// 设置复选框样式和多选功能 - 使用Windows API
    ListView_SetExtendedListViewStyle(_hListCtrl, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    /* create toolbar */
    _ToolBar.init(_hInst, _hSelf, TB_STANDARD, toolBarIcons, sizeof(toolBarIcons) / sizeof(ToolBarButtonUnit));

    _Rebar.init(_hInst, _hSelf);
    _ToolBar.addToRebar(&_Rebar);
    _Rebar.setIDVisible(REBAR_BAR_TOOLBAR, true);
}

void FilterDialog::InitializeListWithCheckboxes()
{
    // 添加列 - 使用Windows API
    LVCOLUMN lvc;
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    
lvc.iSubItem = 0;
    lvc.pszText = (LPWSTR)L"选择";
    lvc.cx = 60;
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(_hListCtrl, 0, &lvc);
    
    lvc.iSubItem = 1;
    lvc.pszText = (LPWSTR)L"过滤器";
    lvc.cx = 200;
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(_hListCtrl, 1, &lvc);
    
    lvc.iSubItem = 2;
    lvc.pszText = (LPWSTR)L"过滤规则";
    lvc.cx = 80;
    lvc.fmt = LVCFMT_RIGHT;
    ListView_InsertColumn(_hListCtrl, 2, &lvc);
    
    lvc.iSubItem = 3;
    lvc.pszText = (LPWSTR)L"使用正则";
    lvc.cx = 60;
    lvc.fmt = LVCFMT_CENTER;
    ListView_InsertColumn(_hListCtrl, 3, &lvc);
    
    // 样式已经在initDialog2方法中设置，包含LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES
    // 不需要重复设置
    
    // 添加示例数据
    //AddItemWithCheckbox((LPWSTR)L"file1.txt", (LPWSTR)L"1.2 KB", (LPWSTR)L"文本文件");
    //AddItemWithCheckbox((LPWSTR)L"image.jpg", (LPWSTR)L"2.5 MB", (LPWSTR)L"图像文件");
    //AddItemWithCheckbox((LPWSTR)L"document.pdf", (LPWSTR)L"3.1 MB", (LPWSTR)L"PDF文档");
}



BOOL FilterDialog::IsItemChecked(int nIndex)
{
    return ListView_GetCheckState(_hListCtrl, nIndex);
}

void FilterDialog::OnCheckboxChanged(int nItemIndex)
{
    BOOL bChecked = ListView_GetCheckState(_hListCtrl, nItemIndex);
    
    // 获取文件名
    TCHAR fileName[256];
    ListView_GetItemText(_hListCtrl, nItemIndex, 1, fileName, 256);
    
    // 处理复选框状态改变的逻辑
    if (bChecked) {
        // 项被选中
    } else {
        // 项被取消选中
    }
}

void FilterDialog::CheckAllItems()
{
    int itemCount = ListView_GetItemCount(_hListCtrl);
    for (int i = 0; i < itemCount; i++) {
        ListView_SetCheckState(_hListCtrl, i, TRUE);
    }
}

void FilterDialog::UncheckAllItems()
{
    int itemCount = ListView_GetItemCount(_hListCtrl);
    for (int i = 0; i < itemCount; i++) {
        ListView_SetCheckState(_hListCtrl, i, FALSE);
    }
}

void FilterDialog::ToggleAllItems()
{
    int itemCount = ListView_GetItemCount(_hListCtrl);
    for (int i = 0; i < itemCount; i++) {
        BOOL currentState = ListView_GetCheckState(_hListCtrl, i);
        ListView_SetCheckState(_hListCtrl, i, !currentState);
    }
}

void FilterDialog::AddItemWithCheckbox(const wchar_t* filterName, const wchar_t* filterRule, bool useRegex)
{
    int nIndex = ListView_GetItemCount(_hListCtrl);
    
    // 插入新项
    LVITEM lvi;
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = nIndex;
    lvi.iSubItem = 0;
    lvi.pszText = (LPWSTR)L"";
    lvi.lParam = (LPARAM)nIndex;
    ListView_InsertItem(_hListCtrl, &lvi);
    
    // 设置其他列的数据
    ListView_SetItemText(_hListCtrl, nIndex, 1, (LPTSTR)filterName);
    ListView_SetItemText(_hListCtrl, nIndex, 2, (LPTSTR)filterRule);
    ListView_SetItemText(_hListCtrl, nIndex, 3, (LPTSTR)(useRegex ? L"1" : L"0"));
    
    // 默认选中该项
    ListView_SetCheckState(_hListCtrl, nIndex, TRUE);
}

void FilterDialog::DeleteSelectedItems()
{
    int itemCount = ListView_GetItemCount(_hListCtrl);
    
    // 从后往前删除，避免索引变化问题
    for (int i = itemCount - 1; i >= 0; i--) {
        if (ListView_GetItemState(_hListCtrl, i, LVIS_SELECTED)) {
            ListView_DeleteItem(_hListCtrl, i);
        }
    }
}

void FilterDialog::doDialog(bool willBeShown)
{
    if (!isCreated()) {
        // define the default docking behaviour
        tTbData data{};
        create(&data);
        data.pszName = L"Filter";
        data.dlgID = DOCKABLE_FILTER_INDEX;
        data.uMask = DWS_DF_CONT_LEFT | DWS_ADDINFO | DWS_ICONTAB;
        //data.hIconTab = (HICON)::LoadImage(_hInst, MAKEINTRESOURCE(IDI_EXPLORE), IMAGE_ICON, 0, 0, LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT);
        data.pszModuleName = getPluginFileName();

        ThemeRenderer::Instance().Register(_hSelf);
        ::SendMessage(_hParent, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
    }
    //else if (willBeShown) {
    //    if (_pExProp->bAutoUpdate == TRUE) {
    //        ::KillTimer(_hSelf, EXT_UPDATEACTIVATE);
    //        ::SetTimer(_hSelf, EXT_UPDATEACTIVATE, 0, nullptr);
    //    }
    //    else {
    //        ::KillTimer(_hSelf, EXT_UPDATEACTIVATEPATH);
    //        ::SetTimer(_hSelf, EXT_UPDATEACTIVATEPATH, 0, nullptr);
    //    }
    //}
    display(willBeShown);
}

INT_PTR CALLBACK FilterDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message) {
    case WM_INITDIALOG: {
        InitialDialog();
        InitializeListWithCheckboxes();  // 初始化带复选框的列表
        break;
    }
    case WM_COMMAND: {
        //if (((HWND)lParam == _hFilter) && (HIWORD(wParam) == CBN_SELCHANGE)) {
        //    ::SendMessage(_hSelf, EXM_CHANGECOMBO, 0, 0);
        //    return TRUE;
        //}
        if ((HWND)lParam == _ToolBar.getHSelf()) {
            tb_cmd(LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->hwndFrom == _hListCtrl) {
            if (pnmh->code == LVN_ITEMCHANGED) {
                LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
                if ((pnmv->uChanged & LVIF_STATE) && (pnmv->uNewState & LVIS_STATEIMAGEMASK)) {
                    // 复选框状态改变
                    OnCheckboxChanged(pnmv->iItem);
                }
            }
            else if (pnmh->code == NM_DBLCLK) {
                // 双击事件处理
                LPNMITEMACTIVATE pnmia = (LPNMITEMACTIVATE)lParam;
                if (pnmia->iItem >= 0) {
                    OnItemDoubleClick(pnmia->iItem);
                }
            }
        }
        //DockingDlgInterface::run_dlgProc(Message, wParam, lParam);
        break;
    }
    case WM_SIZE:
    case WM_MOVE:
        if (_bStartupFinish == FALSE) {
            return TRUE;
        }
        UpdateLayout();
        break;
    //case WM_PAINT:
    //    ::RedrawWindow(_ToolBar.getHSelf(), nullptr, nullptr, TRUE);
    //    break;
    case WM_DESTROY: {
        _ToolBar.destroy();
        /* unsubclass */
        if (_hDefaultListProc != nullptr) {
            ::SetWindowLongPtr(_hListCtrl, GWLP_WNDPROC, (LONG_PTR)_hDefaultListProc);
            _hDefaultListProc = nullptr;
        }
        break;
    }
    default:
        return DockingDlgInterface::run_dlgProc(Message, wParam, lParam);
    }
}

void FilterDialog::tb_cmd(WPARAM message)
{
    switch (message) {
    case IDM_EX_ADD:
        // 创建规则对话框并获取用户输入
        if (createRuleDlg.doDialog() == IDOK) {
            // 获取用户输入的过滤名称和规则
            const wchar_t* title = createRuleDlg.GetTitle();
            const wchar_t* rule = createRuleDlg.GetRule();
            
            // 添加到ListCtrl中
            AddItemWithCheckbox(title, rule, false);
            FilterCurrentDocument();
        }
        break;
    case IDM_EX_DEL:
        // 删除选中的项
        DeleteSelectedItems();
        FilterCurrentDocument();
        break;
    case IDM_EX_SAVEFILTER:
        // 保存过滤规则到文件
        SaveFilterRules();
        break;
    case IDM_EX_IMPORTFILTER:
        // 从文件导入过滤规则
        ImportFilterRules();
        break;
    case IDM_EX_REFRESH:
        // 刷新当前文档过滤
        FilterCurrentDocument();
        break;
    }
}

void FilterDialog::UpdateLayout()
{
    RECT rc = { 0 };
    RECT rcBuff = { 0 };

    getClientRect(rc);

    _ToolBar.reSizeTo(rc);
    _Rebar.reSizeTo(rc);

    auto toolBarHeight = _ToolBar.getHeight();

    getClientRect(rc);
    rc.top += toolBarHeight;
    rc.bottom -= toolBarHeight;

    ::SetWindowPos(_hListCtrl, nullptr, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_SHOWWINDOW);
}

void FilterDialog::redraw()
{
    UpdateLayout();
}

/****************************************************************************
 * Message handling of tree
 */
LRESULT FilterDialog::runListProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    return ::CallWindowProc(_hDefaultListProc, hwnd, Message, wParam, lParam);
}

void FilterDialog::FilterCurrentDocument()
{
    // 高精度计时器变量
    LARGE_INTEGER frequency, startTime, endTime, stepStartTime, stepEndTime;
    double totalTime = 0.0, stepTime = 0.0;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startTime);
    
    // 获取当前Scintilla编辑器句柄
    QueryPerformanceCounter(&stepStartTime);
    UINT currentEdit;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
    HWND currentSciHandle = (0 == currentEdit) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
    QueryPerformanceCounter(&stepEndTime);
    stepTime = (double)(stepEndTime.QuadPart - stepStartTime.QuadPart) / frequency.QuadPart * 1000.0;
    
    if (!currentSciHandle) {
        ::MessageBox(_hSelf, L"无法获取当前编辑器句柄", L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    
    // 获取文档总长度
    QueryPerformanceCounter(&stepStartTime);
    int docLength = (int)::SendMessage(currentSciHandle, SCI_GETLENGTH, 0, 0);
    QueryPerformanceCounter(&stepEndTime);
    stepTime += (double)(stepEndTime.QuadPart - stepStartTime.QuadPart) / frequency.QuadPart * 1000.0;
    
    if (docLength <= 0) {
        ::MessageBox(_hSelf, L"当前文档为空", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }
    
    // 获取所有勾选的过滤规则
    QueryPerformanceCounter(&stepStartTime);
    int itemCount = ListView_GetItemCount(_hListCtrl);
    std::vector<std::wstring> filterRules;
    std::vector<bool> useRegexFlags;
    
    for (int i = 0; i < itemCount; i++) {
        // 只处理被勾选的项
        if (ListView_GetCheckState(_hListCtrl, i)) {
            wchar_t ruleText[256];
            ListView_GetItemText(_hListCtrl, i, 2, ruleText, 256);
            
            if (ruleText[0] != L'\0') {
                // 获取正则表达式选项（从第4列，索引3）
                wchar_t useRegexStr[10];
                bool useRegex = false;
                ListView_GetItemText(_hListCtrl, i, 3, useRegexStr, 10);
                useRegex = (wcscmp(useRegexStr, L"1") == 0);
                
                filterRules.push_back(std::wstring(ruleText));
                useRegexFlags.push_back(useRegex);
            }
        }
    }

    if (filterRules.empty()) {
        ::MessageBox(_hSelf, L"没有定义任何过滤规则", L"提示", MB_OK | MB_ICONINFORMATION);
        return;
    }

    QueryPerformanceCounter(&stepEndTime);
    double filterRulesTime = (double)(stepEndTime.QuadPart - stepStartTime.QuadPart) / frequency.QuadPart * 1000.0;

    //if (filterRules.empty()) {
    //    ::MessageBox(_hSelf, L"没有定义任何过滤规则", L"提示", MB_OK | MB_ICONINFORMATION);
    //    return;
    //}

    // 首先显示所有行并清除之前的标记
    QueryPerformanceCounter(&stepStartTime);
    int totalLines = (int)::SendMessage(currentSciHandle, SCI_GETLINECOUNT, 0, 0);
    ::SendMessage(currentSciHandle, SCI_SHOWLINES, 0, totalLines - 1);

    // 批量清除所有现有的隐藏标记（保留以兼容旧版本清理）
    ::SendMessage(currentSciHandle, SCI_MARKERDELETEALL, 19, 0);
    ::SendMessage(currentSciHandle, SCI_MARKERDELETEALL, 18, 0);

    // 确保折叠边距可见（在纯文本模式下Notepad++可能会隐藏折叠边距）
    int marginWidth = (int)::SendMessage(currentSciHandle, SCI_GETMARGINWIDTHN, 2, 0);
    if (marginWidth == 0) {
        // 如果折叠边距被隐藏，设置一个默认宽度（通常为16）
        ::SendMessage(currentSciHandle, SCI_SETMARGINWIDTHN, 2, 16);
    }
    // 确保边距2用于显示折叠符号并且响应鼠标点击
    ::SendMessage(currentSciHandle, SCI_SETMARGINTYPEN, 2, SC_MARGIN_SYMBOL);
    ::SendMessage(currentSciHandle, SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
    ::SendMessage(currentSciHandle, SCI_SETMARGINSENSITIVEN, 2, 1);

    // 开启Scintilla的折叠属性
    ::SendMessage(currentSciHandle, SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");

    // 显式定义折叠标记样式（加号和减号），以防在纯文本模式下未定义
    ::SendMessage(currentSciHandle, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_MINUS);
    ::SendMessage(currentSciHandle, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PLUS);
    ::SendMessage(currentSciHandle, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_EMPTY);
    ::SendMessage(currentSciHandle, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_EMPTY);
    ::SendMessage(currentSciHandle, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_EMPTY);
    ::SendMessage(currentSciHandle, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_EMPTY);
    ::SendMessage(currentSciHandle, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_EMPTY);

    // 设置折叠标记的颜色为蓝色 (Scintilla颜色格式为BGR: 0xBBGGRR，蓝色为 0xFF0000)
    ::SendMessage(currentSciHandle, SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, 0xFF0000);
    ::SendMessage(currentSciHandle, SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, 0xFF0000);
    ::SendMessage(currentSciHandle, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, 0xFF0000);
    ::SendMessage(currentSciHandle, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, 0xFF0000);
    
    QueryPerformanceCounter(&stepEndTime);
    double clearMarkersTime = (double)(stepEndTime.QuadPart - stepStartTime.QuadPart) / frequency.QuadPart * 1000.0;
    
    // 逐行检查并记录匹配结果
    QueryPerformanceCounter(&stepStartTime);
    int hiddenSections = 0;
    int processedLines = 0;
    double lineProcessingTime = 0.0;
    
    std::vector<bool> matchResults(totalLines, false);
    
    for (int line = 0; line < totalLines; line++) {
        LARGE_INTEGER lineStartTime, lineEndTime;
        QueryPerformanceCounter(&lineStartTime);
        
        int lineLength = (int)::SendMessage(currentSciHandle, SCI_LINELENGTH, line, 0);
        bool matchesAnyRule = false;
        
        if (lineLength > 0) {
            // 直接获取UTF-8文本并转换为宽字符，避免中间char数组
            std::vector<char> lineBuffer(lineLength + 1);
            // 先清零缓冲区，因为SCI_GETLINE不会自动添加null终止符
            memset(lineBuffer.data(), 0, lineBuffer.size());
            ::SendMessage(currentSciHandle, SCI_GETLINE, line, (LPARAM)lineBuffer.data());
            
            // 一次性转换为宽字符，使用实际长度而不是-1
            int wideLength = MultiByteToWideChar(CP_UTF8, 0, lineBuffer.data(), lineLength, nullptr, 0);
            if (wideLength > 0) {
                std::wstring wideLineText(wideLength, L'\0');
                MultiByteToWideChar(CP_UTF8, 0, lineBuffer.data(), lineLength, &wideLineText[0], wideLength);
                
                // 检查行是否匹配任何过滤规则
                for (size_t j = 0; j < filterRules.size(); j++) {
                    const auto& rule = filterRules[j];
                    bool useRegex = useRegexFlags[j];
                    
                    if (useRegex) {
                        // 使用正则表达式匹配
                        try {
                            std::wregex regexPattern(rule);
                            if (std::regex_search(wideLineText, regexPattern)) {
                                matchesAnyRule = true;
                                break;
                            }
                        } catch (const std::regex_error&) {
                            // 正则表达式错误，回退到字符串搜索
                            if (wideLineText.find(rule) != std::wstring::npos) {
                                matchesAnyRule = true;
                                break;
                            }
                        }
                    } else {
                        // 使用字符串搜索
                        if (wideLineText.find(rule) != std::wstring::npos) {
                            matchesAnyRule = true;
                            break;
                        }
                    }
                }
            }
        }
        
        matchResults[line] = matchesAnyRule;
        
        QueryPerformanceCounter(&lineEndTime);
        lineProcessingTime += (double)(lineEndTime.QuadPart - lineStartTime.QuadPart) / frequency.QuadPart * 1000.0;
        processedLines++;
    }
    
    // 设置折叠级别并执行折叠
    for (int line = 0; line < totalLines; line++) {
        bool isMatch = matchResults[line];
        bool nextIsMatch = (line + 1 < totalLines) ? matchResults[line + 1] : true;
        
        int level = SC_FOLDLEVELBASE;
        
        if (!isMatch) {
            if (line == 0) {
                level = SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG;
            } else {
                level = SC_FOLDLEVELBASE + 1;
            }
        } else {
            if (!nextIsMatch) {
                level = SC_FOLDLEVELBASE | SC_FOLDLEVELHEADERFLAG;
            } else {
                level = SC_FOLDLEVELBASE;
            }
        }
        
        ::SendMessage(currentSciHandle, SCI_SETFOLDLEVEL, line, level);
        ::SendMessage(currentSciHandle, SCI_SETFOLDEXPANDED, line, 1);
    }
    
    for (int line = 0; line < totalLines; line++) {
        int level = (int)::SendMessage(currentSciHandle, SCI_GETFOLDLEVEL, line, 0);
        if (level & SC_FOLDLEVELHEADERFLAG) {
            ::SendMessage(currentSciHandle, SCI_FOLDLINE, line, SC_FOLDACTION_CONTRACT);
            hiddenSections++;
        }
    }
    
    QueryPerformanceCounter(&stepEndTime);
    double lineProcessingTotalTime = (double)(stepEndTime.QuadPart - stepStartTime.QuadPart) / frequency.QuadPart * 1000.0;
    
    double hideOperationTime = 0.0; // 隐藏操作已合并到逐行处理中，时间为0
    
    // 计算总耗时
    QueryPerformanceCounter(&endTime);
    totalTime = (double)(endTime.QuadPart - startTime.QuadPart) / frequency.QuadPart * 1000.0;
    
    // 显示详细的耗时统计信息
    wchar_t message[512];
    swprintf_s(message, L"文档过滤完成，标记并隐藏了 %d 个区域\n\n"
                L"耗时统计（优化后）：\n"
                L"总耗时：%.2f 毫秒\n"
                L"- 获取句柄和文档：%.2f 毫秒\n"
                L"- 获取过滤规则：%.2f 毫秒\n"
                L"- 清除标记（批量优化）：%.2f 毫秒\n"
                L"- 逐行处理、标记并隐藏（%d 行）：%.2f 毫秒\n"
                L"  （平均每行：%.3f 毫秒）",
                hiddenSections,
                totalTime,
                stepTime,
                filterRulesTime,
                clearMarkersTime,
                processedLines, lineProcessingTotalTime,
                lineProcessingTime / processedLines);
    ::MessageBox(_hSelf, message, L"成功", MB_OK | MB_ICONINFORMATION);
}

void FilterDialog::SaveFilterRules()
{
    // 弹出保存文件对话框
    OPENFILENAME ofn;
    wchar_t szFile[260] = L"filter_rules.txt";
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = _hSelf;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileName(&ofn) == TRUE) {
        // 获取ListCtrl中的所有项目
        int itemCount = ListView_GetItemCount(_hListCtrl);
        if (itemCount == 0) {
            MessageBox(_hSelf, L"没有过滤规则可保存", L"提示", MB_OK | MB_ICONINFORMATION);
            return;
        }
        
        // 创建文件并写入数据
        HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            MessageBox(_hSelf, L"无法创建文件", L"错误", MB_OK | MB_ICONERROR);
            return;
        }
        
        // 写入文件头
        const char* header = "# Filter Rules Export\r\n# Format: FilterName|FilterRule|UseRegex\r\n";
        DWORD bytesWritten;
        WriteFile(hFile, header, (DWORD)strlen(header), &bytesWritten, NULL);
        
        // 写入每个过滤规则
        for (int i = 0; i < itemCount; i++) {
            wchar_t filterName[256];
            wchar_t filterRule[256];
            wchar_t useRegexStr[10];
            
            ListView_GetItemText(_hListCtrl, i, 1, filterName, 256);
            ListView_GetItemText(_hListCtrl, i, 2, filterRule, 256);
            ListView_GetItemText(_hListCtrl, i, 3, useRegexStr, 10);
            
            // 转换为UTF-8格式写入
            int nameLen = WideCharToMultiByte(CP_UTF8, 0, filterName, -1, NULL, 0, NULL, NULL);
            int ruleLen = WideCharToMultiByte(CP_UTF8, 0, filterRule, -1, NULL, 0, NULL, NULL);
            int regexLen = WideCharToMultiByte(CP_UTF8, 0, useRegexStr, -1, NULL, 0, NULL, NULL);
            
            if (nameLen > 0 && ruleLen > 0) {
                std::vector<char> nameBuffer(nameLen);
                std::vector<char> ruleBuffer(ruleLen);
                std::vector<char> regexBuffer(regexLen);
                
                WideCharToMultiByte(CP_UTF8, 0, filterName, -1, nameBuffer.data(), nameLen, NULL, NULL);
                WideCharToMultiByte(CP_UTF8, 0, filterRule, -1, ruleBuffer.data(), ruleLen, NULL, NULL);
                WideCharToMultiByte(CP_UTF8, 0, useRegexStr, -1, regexBuffer.data(), regexLen, NULL, NULL);
                
                std::string line = std::string(nameBuffer.data()) + "|" + std::string(ruleBuffer.data()) + "|" + std::string(regexBuffer.data()) + "\r\n";
                WriteFile(hFile, line.c_str(), (DWORD)line.length(), &bytesWritten, NULL);
            }
        }
        
        CloseHandle(hFile);
        
        wchar_t message[256];
        swprintf_s(message, L"成功保存 %d 个过滤规则到文件", itemCount);
        MessageBox(_hSelf, message, L"保存成功", MB_OK | MB_ICONINFORMATION);
    }
}

void FilterDialog::ImportFilterRules()
{
    // 弹出打开文件对话框
    OPENFILENAME ofn;
    wchar_t szFile[260] = L"";
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = _hSelf;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn) == TRUE) {
        // 导入前先清空_hListCtrl中的所有项目
        ListView_DeleteAllItems(_hListCtrl);
        
        // 读取文件内容
        HANDLE hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            MessageBox(_hSelf, L"无法打开文件", L"错误", MB_OK | MB_ICONERROR);
            return;
        }
        
        DWORD fileSize = GetFileSize(hFile, NULL);
        if (fileSize == INVALID_FILE_SIZE) {
            CloseHandle(hFile);
            MessageBox(_hSelf, L"无法获取文件大小", L"错误", MB_OK | MB_ICONERROR);
            return;
        }
        
        std::vector<char> buffer(fileSize + 1);
        DWORD bytesRead;
        if (!ReadFile(hFile, buffer.data(), fileSize, &bytesRead, NULL)) {
            CloseHandle(hFile);
            MessageBox(_hSelf, L"读取文件失败", L"错误", MB_OK | MB_ICONERROR);
            return;
        }
        buffer[fileSize] = '\0';
        CloseHandle(hFile);
        
        // 解析文件内容
        std::string content(buffer.data());
        std::istringstream iss(content);
        std::string line;
        int importedCount = 0;
        
        while (std::getline(iss, line)) {
            // 跳过空行和注释行
            if (line.empty() || line[0] == '#') {
                continue;
            }
            
            // 移除行尾的\r字符（Windows换行符处理）
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            // 解析格式：FilterName|FilterRule|UseRegex（兼容老格式FilterName|FilterRule）
            size_t firstPos = line.find('|');
            if (firstPos != std::string::npos) {
                std::string nameStr = line.substr(0, firstPos);
                
                // 查找第二个分隔符
                size_t secondPos = line.find('|', firstPos + 1);
                std::string ruleStr;
                std::string useRegexStr = "0"; // 默认不使用正则表达式
                
                if (secondPos != std::string::npos) {
                    // 新格式：FilterName|FilterRule|UseRegex
                    ruleStr = line.substr(firstPos + 1, secondPos - firstPos - 1);
                    useRegexStr = line.substr(secondPos + 1);
                } else {
                    // 老格式：FilterName|FilterRule
                    ruleStr = line.substr(firstPos + 1);
                }
                
                // 转换为宽字符
                int nameLen = MultiByteToWideChar(CP_UTF8, 0, nameStr.c_str(), -1, NULL, 0);
                int ruleLen = MultiByteToWideChar(CP_UTF8, 0, ruleStr.c_str(), -1, NULL, 0);
                int regexLen = MultiByteToWideChar(CP_UTF8, 0, useRegexStr.c_str(), -1, NULL, 0);
                
                if (nameLen > 0 && ruleLen > 0) {
                    std::vector<wchar_t> nameBuffer(nameLen);
                    std::vector<wchar_t> ruleBuffer(ruleLen);
                    std::vector<wchar_t> regexBuffer(regexLen);
                    
                    MultiByteToWideChar(CP_UTF8, 0, nameStr.c_str(), -1, nameBuffer.data(), nameLen);
                    MultiByteToWideChar(CP_UTF8, 0, ruleStr.c_str(), -1, ruleBuffer.data(), ruleLen);
                    MultiByteToWideChar(CP_UTF8, 0, useRegexStr.c_str(), -1, regexBuffer.data(), regexLen);
                    
                    // 添加到ListCtrl中，并设置正则表达式选项
                    bool useRegex = (wcscmp(regexBuffer.data(), L"1") == 0);
                    AddItemWithCheckbox(nameBuffer.data(), ruleBuffer.data(), useRegex);
                    
                    importedCount++;
                }
            }
        }
        FilterCurrentDocument();
        wchar_t message[256];
        swprintf_s(message, L"成功导入 %d 个过滤规则", importedCount);
        MessageBox(_hSelf, message, L"导入成功", MB_OK | MB_ICONINFORMATION);
    }
}

// FilterDialog 消息处理程序

void FilterDialog::OnItemDoubleClick(int nItemIndex)
{
    EditFilterItem(nItemIndex);
}

void FilterDialog::EditFilterItem(int nItemIndex)
{
    if (nItemIndex < 0 || nItemIndex >= ListView_GetItemCount(_hListCtrl)) {
        return;
    }
    
    // 获取当前选中项的数据
    wchar_t filterName[256];
    wchar_t filterRule[256];
    
    ListView_GetItemText(_hListCtrl, nItemIndex, 1, filterName, 256);
    ListView_GetItemText(_hListCtrl, nItemIndex, 2, filterRule, 256);
    
    // 获取正则表达式选项（从第4列，索引3）
    wchar_t useRegexStr[10];
    bool useRegex = false;
    ListView_GetItemText(_hListCtrl, nItemIndex, 3, useRegexStr, 10);
    useRegex = (wcscmp(useRegexStr, L"1") == 0);
    
    // 设置编辑对话框的初始值
    createRuleDlg.SetInitialValues(filterName, filterRule, useRegex);
    
    // 显示编辑对话框
    if (createRuleDlg.doDialog() == IDOK) {
        // 获取用户修改后的数据
        const wchar_t* newTitle = createRuleDlg.GetTitle();
        const wchar_t* newRule = createRuleDlg.GetRule();
        bool newUseRegex = createRuleDlg.GetUseRegex();
        
        // 更新ListCtrl中的数据
        ListView_SetItemText(_hListCtrl, nItemIndex, 1, (LPTSTR)newTitle);
        ListView_SetItemText(_hListCtrl, nItemIndex, 2, (LPTSTR)newRule);
        ListView_SetItemText(_hListCtrl, nItemIndex, 3, (LPTSTR)(newUseRegex ? L"1" : L"0"));
        
        // 重新过滤当前文档
        FilterCurrentDocument();
    }
}