#pragma once
#include "DockingFeature/StaticDialog.h"

class CreateRuleDialog : public StaticDialog
{

public:
    CreateRuleDialog();
    ~CreateRuleDialog();

    INT_PTR doDialog();

    void destroy() override {}

    // 获取用户输入的数据
    const wchar_t* GetTitle() const { return _title; }
    const wchar_t* GetRule() const { return _rule; }
    bool GetUseRegex() const { return _useRegex; }

    // 设置初始值用于编辑模式
    void SetInitialValues(const wchar_t* title, const wchar_t* rule, bool useRegex = false);

protected:
    INT_PTR CALLBACK run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam) override;

    void LongUpdate();
    void SetParams();
    BOOL GetParams();

private:
    wchar_t _title[256];
    wchar_t _rule[256];
    bool _isEditMode;
    bool _useRegex;
};