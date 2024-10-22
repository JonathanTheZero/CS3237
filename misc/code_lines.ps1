# Simple script to count the code lines in a given repository
# just to get some analytics

$rootDir = "..\"

$excludeFolders = @("Reports", ".git", "logs")

$files = Get-ChildItem -Path $rootDir -Recurse -File -Force | 
Where-Object {
    # check if the folder should be included or not
    $exclude = $false
    foreach ($folder in $excludeFolders) {
        if ($_.FullName -like "*\$folder\*") {
            $exclude = $true
            break
        }
    }
    return -not $exclude
}

$lineCounts = @{}

foreach ($file in $files) {
    $ext = $file.Extension.ToLower()

    $lineCount = (Get-Content -Path $file.FullName | Measure-Object -Line).Lines

    if ($lineCounts.ContainsKey($ext)) {
        $lineCounts[$ext] += $lineCount
    }
    else {
        $lineCounts[$ext] = $lineCount
    }
}

$lineCounts.GetEnumerator() | Sort-Object Value -Descending | ForEach-Object {
    Write-Output ("Extension: {0}, Total Lines: {1}" -f $_.Key, $_.Value)
}

Write-Host "Press Enter to exit..." -ForegroundColor Yellow
[void][System.Console]::ReadLine()