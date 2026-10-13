# Squad Roguelike — local test launcher (used by the desktop shortcut).
#   1. makes sure Docker + the Nakama stack are up
#   2. starts N headless companion bots (a squad needs 4 players)
#   3. opens the game window and, when it closes, stops the bots
# Usage: launcher.ps1 [-Bots 3]   (-Bots 0 to play with real friends)
param([int]$Bots = 3)

# Native tools (docker) write warnings to stderr; with 'Stop' PowerShell 5.1 would
# turn those into exceptions, so rely on exit codes instead.
$ErrorActionPreference = 'Continue'
Add-Type -AssemblyName System.Windows.Forms

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$compose = Join-Path $root '..\Nakama_Legacy_Reference\docker-compose.yml'
$game = Join-Path $root 'SquadRoguelike.exe'
$companions = @('crusader:support', 'mage:magic', 'barbarian:defense')

function Fail([string]$message) {
    [System.Windows.Forms.MessageBox]::Show($message, 'Squad Roguelike', 'OK', 'Error') | Out-Null
    exit 1
}

try {
    docker info 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) {
        docker desktop start --timeout 180 2>&1 | Out-Null
        docker info 2>&1 | Out-Null
        if ($LASTEXITCODE -ne 0) { Fail 'Não consegui iniciar o Docker Desktop. Abra-o manualmente e tente de novo.' }
    }
    docker compose -f $compose up -d 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) { Fail 'Falha ao subir o servidor Nakama (docker compose up).' }
    $healthy = $false
    for ($i = 0; $i -lt 60 -and -not $healthy; $i++) {
        $healthy = (docker compose -f $compose ps nakama --format '{{.Health}}' 2>$null) -eq 'healthy'
        if (-not $healthy) { Start-Sleep -Seconds 2 }
    }
    if (-not $healthy) { Fail 'O servidor Nakama não ficou pronto a tempo.' }
} catch {
    Fail "Docker não encontrado ou com erro: $($_.Exception.Message)"
}

$bots = @()
for ($i = 0; $i -lt [Math]::Min($Bots, $companions.Count); $i++) {
    $botArgs = @('--headless', '--', "--profile=companion$($i + 1)", "--autoplay=$($companions[$i])", '--autoplay-loop', '--exit-with-player')
    $bots += Start-Process -FilePath $game -ArgumentList $botArgs -WindowStyle Hidden -PassThru
}

try {
    $player = Start-Process -FilePath $game -ArgumentList @('--', '--profile=player') -PassThru
    $player.WaitForExit()
} finally {
    foreach ($bot in $bots) {
        if (-not $bot.HasExited) { Stop-Process -Id $bot.Id -Force -ErrorAction SilentlyContinue }
    }
}
