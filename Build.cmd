@echo off
if not exist "%VS100COMNTOOLS%vsvars32.bat" (
    echo Cannot found Visual Studio 2010 Toolset.
    echo exiting...
    goto :end
)

call "%VS100COMNTOOLS%vsvars32.bat"
devenv /build Release foo_uie_wsh_panel_mod_plus.sln

:end