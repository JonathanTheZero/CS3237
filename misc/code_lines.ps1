# Simple script to count the code lines and file sizes in a given repository for analytics

$rootDir = "..\"
# $excludeFolders = @("Reports", ".git", "logs", "Lab3\ipynb-example", "Lab4\examples")
$excludeFolders = @("Reports", ".git", "logs")


$files = Get-ChildItem -Path $rootDir -Recurse -File -Force | 
Where-Object {
    # Check if the folder should be included or not
    $exclude = $false
    foreach ($folder in $excludeFolders) {
        if ($_.FullName -like "*\$folder\*") {
            $exclude = $true
            break
        }
    }
    return -not $exclude
}


$stats = @{}
$totalLines = 0
$totalSize = 0


foreach ($file in $files) {
    $ext = $file.Extension.ToLower()

    $lineCount = (Get-Content -Path $file.FullName | Measure-Object -Line).Lines
    $fileSize = $file.Length
    $totalLines += $lineCount
    $totalSize += $fileSize

    if ($stats.ContainsKey($ext)) {
        $stats[$ext]["LineCount"] += $lineCount
        $stats[$ext]["FileSize"] += $fileSize
    }
    else {
        $stats[$ext] = @{
            "LineCount" = $lineCount
            "FileSize"  = $fileSize
        }
    }
}


$stats.GetEnumerator() | Sort-Object { $_.Value["LineCount"] } -Descending | 
Select-Object @{Name = "Extension"; Expression = { $_.Key } }, 
@{Name = "Total Lines"; Expression = { $_.Value["LineCount"] } }, 
@{Name = "Lines %"; Expression = { [math]::round(($_.Value["LineCount"] / $totalLines) * 100, 2) } }, 
@{Name = "Total Size (KB)"; Expression = { [math]::round($_.Value["FileSize"] / 1KB, 2) } },
@{Name = "Size %"; Expression = { [math]::round(($_.Value["FileSize"] / $totalSize) * 100, 2) } } | 
Format-Table -AutoSize


Write-Host "Press Enter to exit..." -ForegroundColor Yellow
[void][System.Console]::ReadLine()
