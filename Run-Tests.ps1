Clear-Host

$root = Join-Path $PSScriptRoot "out/build/x64-Debug/test"

$testExecutables = Get-ChildItem -Path $root -Recurse -File -Filter "*-test.exe" |
    Sort-Object FullName

foreach ($exe in $testExecutables) {
    Write-Host "Running $($exe.FullName)..."

    & $exe.FullName
    $exitCode = $LASTEXITCODE

    if ($exitCode -ne 0) {
        Write-Host "Stopping due to test failure."
        exit $exitCode
    }
}

Write-Host "All tests passed."
exit 0