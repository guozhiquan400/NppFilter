#include <windows.h>
#include "CreateRuleDialog.h"
#include <commctrl.h>
#include <shlobj.h>
#include <commdlg.h>
#include <Shlwapi.h>
#include <array>
#include "DockingFeature/resource.h"

CreateRuleDialog::CreateRuleDialog()
    : StaticDialog()
    , _isEditMode(false)
    , _useRegex(false)
{
    // 初始化字符数组为空字符串
    _title[0] = L'\0';
    _rule[0] = L'\0';
}

CreateRuleDialog::~CreateRuleDialog()
{
}

INT_PTR CreateRuleDialog::doDialog()
{
    return ::DialogBoxParam(_hInst, MAKEINTRESOURCE(IDD_CREATEORMODIFY_FILTER), _hParent, (DLGPROC)dlgProc, (LPARAM)this);
}


INT_PTR CALLBACK CreateRuleDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message) 
    {
        case WM_INITDIALOG:
        {
            goToCenter();
            // 初始化对话框控件
            if (_isEditMode) {
                ::SetDlgItemText(_hSelf, IDC_EDIT_TITLE, _title);
                ::SetDlgItemText(_hSelf, IDC_FILTER_RULE, _rule);
                ::CheckDlgButton(_hSelf, IDC_USE_REGEX, _useRegex ? BST_CHECKED : BST_UNCHECKED);
            } else {
                ::SetDlgItemText(_hSelf, IDC_EDIT_TITLE, L"");
                ::SetDlgItemText(_hSelf, IDC_FILTER_RULE, L"");
                ::CheckDlgButton(_hSelf, IDC_USE_REGEX, BST_UNCHECKED);
            }
            return TRUE;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam)) 
            {
                case IDC_USE_REGEX:
                    // 更新复选框状态
                    _useRegex = (::IsDlgButtonChecked(_hSelf, IDC_USE_REGEX) == BST_CHECKED);
                    return TRUE;
                case IDCANCEL:
                    ::EndDialog(_hSelf, IDCANCEL);
                    return TRUE;
                case IDOK:
                    if (GetParams()) 
                    {
                        ::EndDialog(_hSelf, IDOK);
                    }
                    return TRUE;
            }
        }
        return FALSE;
    }
    return FALSE;
}

BOOL CreateRuleDialog::GetParams()
{
    // 获取标题
    if (!::GetDlgItemText(_hSelf, IDC_EDIT_TITLE, _title, 256)) {
        _title[0] = L'\0';
    }
    
    // 获取规则
    if (!::GetDlgItemText(_hSelf, IDC_FILTER_RULE, _rule, 256)) {
        _rule[0] = L'\0';
    }
    
    // 获取正则表达式选项
    _useRegex = (::IsDlgButtonChecked(_hSelf, IDC_USE_REGEX) == BST_CHECKED);
    
    return TRUE;
}

void CreateRuleDialog::SetInitialValues(const wchar_t* title, const wchar_t* rule, bool useRegex)
{
    _isEditMode = true;
    wcscpy_s(_title, 256, title);
    wcscpy_s(_rule, 256, rule);
    _useRegex = useRegex;
}

//void CreateRuleDialog::LongUpdate()
//{
//    // 长时间更新操作，如果需要的话
//}

//void CreateRuleDialog::SetParams()
//{
//    // 设置对话框参数，如果需要的话
//}