# Rebuilds everything the desktop shortcut runs:
#   Nakama_Legacy_Reference/build/index.js (server module, reloaded into Docker)
#   build/SquadRoguelike.pck (game resources) next to build/SquadRoguelike.exe (Godot runtime)
# Usage: tools\build.ps1 -Godot "C:\path\to\Godot_v4.7.2-stable_win64_console.exe"
param([string]$Godot = "$PSScriptRoot\..\build\SquadRoguelike.exe")

$ErrorActionPreference = 'Stop'
$root = Resolve-Path "$PSScriptRoot\.."

Push-Location "$root\Nakama_Legacy_Reference"
try {
    npm run build
    docker compose restart nakama
} finally { Pop-Location }

# --export-pack only packs resources, so no export templates are needed.
& $Godot --headless --path "$root\Godot_Legacy_Reference" --export-pack "Windows Desktop" "$root\build\SquadRoguelike.pck"
if ($LASTEXITCODE -ne 0) { throw "Godot export failed ($LASTEXITCODE)" }
Write-Host "Build pronto em $root\build"
