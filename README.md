# BioDyNaMo Qt 6.5 Gui

### Building release version on Windows

1. Build Release target in Qt Creator
2. Click on shortcut in your start menu (added during Qt Creator installation) called `Qt 6.x.x (MinGW xx.x.x 64-bit)` (for example `Qt 6.5.0 (MinGW 11.2.0 64-bit)`)
to enable CMD terminal with preconfigured environment
    - Or run `C:\Windows\System32\cmd.exe /A /Q /K C:\Qt\6.5.0\mingw_64\bin\qtenv2.bat` manually
3. Run `QTDIR\bin\windeployqt.exe \your\release\target\directory\with\your_application.exe` where QTDIR is directory to your Qt installation, for example
`C:\Qt\6.5.0\mingw_64\`
4. Executable file should now have all required .dll files copied to it's directory and should run properly

### Building release version on MacOs

1. Build release target
2. Go to release target directory (because path determines final file name)
3. Run `T_INSTALLATION_DIR/6.5.0/macos/bin/macdeployqt /your_application.app -dmg`
