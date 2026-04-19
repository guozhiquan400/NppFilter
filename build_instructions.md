# Notepad++插件编译指南

## 前置步骤：关闭正在运行的Notepad++
在编译前，如果Notepad++正在运行，需要先强制关闭：
```bash
# 检查并关闭Notepad++进程（推荐使用PowerShell版本）
Start-Process PowerShell -ArgumentList "-Command `"Stop-Process -Name 'Notepad++' -Force`"" -Verb RunAs

# 备选方案：如果上述命令无效，尝试管理员权限的taskkill
Start-Process cmd -ArgumentList "/c taskkill /f /im Notepad++.exe" -Verb RunAs
```

## Debug版本编译命令

### 使用Visual Studio 2022 MSBuild编译：
```bash
cmd /c "D:\Soft\Visual2022\MSBuild\Current\Bin\MSBuild.exe e:\notepad-plus-plus-master\plugintemplate-2025_December_02.25.1\vs.proj\NppPluginTemplate.vcxproj /p:Configuration=Debug /p:Platform=x64"
```

### 工作目录：
```
e:\notepad-plus-plus-master
```

## 编译输出
- 生成的DLL文件：`plugintemplate-2025_December_02.25.1\bin64\NppPluginTemplate.dll`
- 自动复制到Notepad++插件目录：`E:\notepad-plus-plus-master\PowerEditor\visual.net\x64\Debug\plugins\NppPluginTemplate\`

## 注意事项
- 需要Visual Studio 2022环境
- 编译后会自动执行PostBuildEvent，复制DLL文件
- 编译过程中可能出现编码警告，但不影响功能
- 确保Visual Studio 2022路径正确：`D:\Soft\Visual2022\`

## 快速验证
编译完成后，可以运行Notepad++验证插件是否加载成功：
```bash
E:\notepad-plus-plus-master\PowerEditor\visual.net\x64\Debug\Notepad++.exe
```