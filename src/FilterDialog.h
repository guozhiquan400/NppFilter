#ifndef FILTER_DLG_H
#define FILTER_DLG_H

#include "DockingFeature/DockingDlgInterface.h"
#include "ToolBar.h"
constexpr INT DOCKABLE_FILTER_INDEX = 0;

// FilterDialog 对话框
class FilterDialog : public DockingDlgInterface
{

public:
	FilterDialog();   // 标准构造函数
	virtual ~FilterDialog();
	void init(HINSTANCE hInst, HWND hParent);
	void InitialDialog();
	void doDialog(bool willBeShown = true);
	void initFinish() {
		_bStartupFinish = TRUE;
		::SendMessage(_hSelf, WM_SIZE, 0, 0);
	};
	void redraw();
	
// ListCtrl复选框相关方法
	void InitializeListWithCheckboxes();
	void AddItemWithCheckbox(const wchar_t* filterName, const wchar_t* filterRule, bool useRegex = false);
	BOOL IsItemChecked(int nIndex);
	void OnCheckboxChanged(int nItemIndex);
	void CheckAllItems();
	void UncheckAllItems();
	void ToggleAllItems();
	void DeleteSelectedItems();
	void FilterCurrentDocument();
	void SaveFilterRules();
	void ImportFilterRules();

	// 双击事件处理方法
	void OnItemDoubleClick(int nItemIndex);
	void EditFilterItem(int nItemIndex);

	LRESULT runListProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK wndDefaultListProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
		return (((FilterDialog*)(::GetWindowLongPtr(hwnd, GWLP_USERDATA)))->runListProc(hwnd, Message, wParam, lParam));
	};
	
protected:
	virtual INT_PTR CALLBACK run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam) override;
	void UpdateLayout();
	void tb_cmd(WPARAM message);
private:
	/* Handles */
	BOOL        _bStartupFinish;
	/* control process */
	WNDPROC     _hDefaultListProc;
	/* handles of controls */
	HWND        _hListCtrl;
	/* classes */
	ToolBar     _ToolBar;
	ReBar       _Rebar;
};
#endif