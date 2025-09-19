@ECHO OFF

echo Unreal Cleanup Script
echo Client directory: %~dp0
pushd "%~dp0"

@RD /S /Q "Intermediate"
@RD /S /Q "Binaries"
@RD /S /Q "DerivedDataCache"
@RD /S /Q "Saved"
del /S /Q *.sln

for /R "Plugins" %%d in (.) do (
    if exist "%%d\Intermediate" rd /S /Q "%%d\Intermediate"
    if exist "%%d\Binaries" rd /S /Q "%%d\Binaries"
)

echo Project cleaned, Exiting
popd