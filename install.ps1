$ErrorActionPreference = 'Stop'

function Find-GitBash {
    $candidates = @()
    $git = Get-Command git.exe -CommandType Application -ErrorAction SilentlyContinue
    if ($git) {
        $gitRoot = Split-Path (Split-Path $git.Source -Parent) -Parent
        $candidates += (Join-Path $gitRoot 'bin\bash.exe')
        $candidates += (Join-Path $gitRoot 'usr\bin\bash.exe')
    }
    $candidates += @(
        (Join-Path $env:ProgramFiles 'Git\bin\bash.exe'),
        (Join-Path $env:ProgramFiles 'Git\usr\bin\bash.exe')
    )
    if (${env:ProgramFiles(x86)}) {
        $candidates += (Join-Path ${env:ProgramFiles(x86)} 'Git\bin\bash.exe')
    }
    if ($env:LOCALAPPDATA) {
        $candidates += (Join-Path $env:LOCALAPPDATA 'Programs\Git\bin\bash.exe')
    }

    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }
    $bash = Get-Command bash.exe -CommandType Application -ErrorAction SilentlyContinue
    if ($bash) { return $bash.Source }
    throw 'Git Bash was not found. Install Git for Windows, then run this installer again.'
}

function ConvertTo-BashPath([string] $Path, [string] $BashExe) {
    $result = & $BashExe --noprofile --norc -c 'cygpath -u "$1"' -- $Path
    if ($LASTEXITCODE -ne 0 -or -not $result) {
        throw "Could not convert path for Git Bash: $Path"
    }
    return ($result | Out-String).Trim()
}

function ConvertTo-BashLiteral([string] $Value) {
    return "'" + $Value.Replace("'", "'\''") + "'"
}

function Find-Compiler([string[]] $Names, [string[]] $Candidates) {
    foreach ($name in $Names) {
        $command = Get-Command $name -CommandType Application -ErrorAction SilentlyContinue
        if ($command) { return $command.Source }
    }
    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }
    return $null
}

try {
    $root = (Resolve-Path -LiteralPath $PSScriptRoot).Path
    $shellDir = Join-Path $root 'shell'
    $libraryDir = Join-Path $root 'library'
    $bashExe = Find-GitBash

    $programFilesRoots = @($env:ProgramFiles, ${env:ProgramFiles(x86)}) | Where-Object { $_ }
    $cxxCandidates = @()
    foreach ($programFilesRoot in $programFilesRoots) {
        $cxxCandidates += (Join-Path $programFilesRoot 'CodeBlocks\MinGW\bin\g++.exe')
    }
    $cxxCandidates += @(
        'C:\MinGW\bin\g++.exe',
        'C:\msys64\mingw64\bin\g++.exe',
        'C:\msys64\ucrt64\bin\g++.exe'
    )
    $cxx = Find-Compiler @('g++.exe', 'g++') $cxxCandidates
    if (-not $cxx) {
        throw 'MinGW g++ was not found. Install a MinGW GCC toolchain and add g++.exe to PATH, or install Code::Blocks with MinGW.'
    }

    $cc = Find-Compiler @('gcc.exe', 'gcc') @()
    if (-not $cc) {
        $cxxDirectory = Split-Path $cxx -Parent
        $possibleCc = Join-Path $cxxDirectory 'gcc.exe'
        if (Test-Path -LiteralPath $possibleCc -PathType Leaf) { $cc = $possibleCc }
        else { $cc = $cxx }
    }

    foreach ($commandFile in @('codeheader', 'mkcpp', 'mkpy', 'rr', 'ccb', 'mkt', 'sb', 'ff')) {
        $commandPath = Join-Path $shellDir $commandFile
        if (-not (Test-Path -LiteralPath $commandPath -PathType Leaf)) {
            throw "Missing command: $commandPath"
        }
    }

    $author = Read-Host 'Enter your name'
    if ([string]::IsNullOrWhiteSpace($author)) { throw 'Name cannot be empty.' }

    $bashRoot = ConvertTo-BashPath $root $bashExe
    $bashShell = ConvertTo-BashPath $shellDir $bashExe
    $bashLibrary = ConvertTo-BashPath $libraryDir $bashExe
    $bashCxx = ConvertTo-BashPath $cxx $bashExe
    $bashCc = ConvertTo-BashPath $cc $bashExe
    $bashAuthor = ConvertTo-BashLiteral $author
    $bashRootLiteral = ConvertTo-BashLiteral $bashRoot
    $bashLibraryLiteral = ConvertTo-BashLiteral $bashLibrary
    $bashShellLiteral = ConvertTo-BashLiteral $bashShell
    $bashCxxLiteral = ConvertTo-BashLiteral $bashCxx
    $bashCcLiteral = ConvertTo-BashLiteral $bashCc

    $begin = '# >>> competitive-programming tools >>>'
    $end = '# <<< competitive-programming tools <<<'
    $bashHome = & $bashExe --noprofile --norc -c 'cygpath -w "$HOME"'
    if ($LASTEXITCODE -ne 0 -or -not $bashHome) { throw 'Could not locate the Git Bash home directory.' }
    $bashHome = ($bashHome | Out-String).Trim()
    $profilePath = Join-Path $bashHome '.bashrc'

    $existingLines = @()
    if (Test-Path -LiteralPath $profilePath -PathType Leaf) {
        $insideBlock = $false
        foreach ($line in [System.IO.File]::ReadAllLines($profilePath)) {
            if ($line -ceq $begin) { $insideBlock = $true; continue }
            if ($line -ceq $end) { $insideBlock = $false; continue }
            if (-not $insideBlock) { $existingLines += $line }
        }
    }

    $block = @(
        $begin,
        "export CP_TOOLS_ROOT=$bashRootLiteral",
        "export CP_LIBRARY=$bashLibraryLiteral",
        "export CP_INCLUDE=$bashLibraryLiteral",
        "export CP_TEMPLATE=$bashLibraryLiteral/default_code.cpp",
        "export CP_AUTHOR=$bashAuthor",
        "export CXX=$bashCxxLiteral",
        "export CC=$bashCcLiteral",
        "export PATH=${bashShellLiteral}:`$PATH",
        $end
    )
    $allLines = @($existingLines) + '' + $block
    $profileText = [string]::Join([Environment]::NewLine, $allLines) + [Environment]::NewLine
    $utf8NoBom = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($profilePath, $profileText, $utf8NoBom)

    $bashCommandFiles = @('codeheader', 'mkcpp', 'mkpy', 'rr', 'ccb', 'mkt', 'sb', 'ff')
    $bashFileArgs = ($bashCommandFiles | ForEach-Object { ConvertTo-BashLiteral "$bashShell/$_" }) -join ' '
    & $bashExe --noprofile --norc -c "chmod +x $bashFileArgs"
    if ($LASTEXITCODE -ne 0) { throw 'Could not mark the shell commands as executable.' }

    Write-Host "> Configured Windows commands in $profilePath"
    Write-Host '> Open Git Bash and run: source ~/.bashrc'
    Write-Host '> Git Bash is required to run these Bash-based tools.'
}
catch {
    Write-Error $_
    exit 1
}
