@echo off
@cls
dotnet publish -p:PublishSingleFile=true --no-self-contained -r win7-x64 -c Release
pause