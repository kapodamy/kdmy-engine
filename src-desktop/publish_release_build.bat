@echo off
@cls
dotnet publish -p:PublishSingleFile=true --no-self-contained -r win-x64 -c Release
pause