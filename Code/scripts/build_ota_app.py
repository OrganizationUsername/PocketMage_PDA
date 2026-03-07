#!/usr/bin/env python3
"""Build OTA App firmware for PocketMage platform."""

import os
import sys
import subprocess
import shutil
from pathlib import Path
from rich.console import Console
from rich.panel import Panel
from rich.progress import Progress

def load_ota_app_properties(properties_path):
    if not properties_path.exists():
        raise FileNotFoundError(f"OTA app properties file not found: {properties_path}")
    props = {}
    with open(properties_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):  # skip comments and empty lines
                continue
            if '=' in line:
                key, value = line.split('=', 1)
                props[key.strip()] = value.strip()
    return props

def main():
    console = Console()
    workspace_root = Path(__file__).parent.parent
    os.chdir(workspace_root)
    ota_app_dir = workspace_root / "PocketMage_V3"
    build_dir = ota_app_dir / ".pio" / "build"

    console.print(Panel("PocketMage OTA App Build", style="bold green"))

    console.print("[bold]Installing PlatformIO...[/bold]")
    result = subprocess.run(["python3", "-m", "pip", "install", "--upgrade", "platformio"])
    if result.returncode != 0:
        console.print("[red]PlatformIO installation failed[/red]")
        sys.exit(1)
    console.print("[green]PlatformIO installed[/green]")

    console.print("[bold]Building OTA App with PlatformIO...[/bold]")
    result = subprocess.run(["pio", "run", "-e", "OTA_APP"], cwd=ota_app_dir)
    if result.returncode != 0:
        console.print("[red]PlatformIO OTA App build failed[/red]")
        sys.exit(1)
    console.print("[green]OTA App Build completed[/green]")

    firmware_files = list(build_dir.glob("*/firmware.bin"))
    if not firmware_files:
        console.print("[red]firmware.bin not found in build directory[/red]")
        sys.exit(1)

    firmware_src = firmware_files[0]
    ota_app_properties = ota_app_dir / "ota_app.properties"
    try:
        metadata = load_ota_app_properties(ota_app_properties)
    except Exception as e:
        console.print(f"[red]Error loading ota_app.properties: {e}[/red]")
        sys.exit(1)

    app_name = metadata.get("name", "OTAApp")
    icon_path = metadata.get("icon", None)
    firmware_dst = ota_app_dir / "ota_build" / f"{app_name}.bin"
    firmware_dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(firmware_src, firmware_dst)
    console.print(f"[green]Copied OTA App firmware as {firmware_dst.name}[/green]")

    # Copy icon if specified
    if icon_path:
        icon_src = ota_app_dir / icon_path
        if icon_src.exists():
            icon_dst = firmware_dst.parent / icon_src.name
            shutil.copy2(icon_src, icon_dst)
            console.print(f"[green]Copied icon: {icon_dst.name}[/green]")
        else:
            console.print(f"[yellow]Icon not found: {icon_src}[/yellow]")

    # Bundle as .tar
    tar_path = firmware_dst.parent / f"{app_name}.tar"
    shutil.make_archive(str(tar_path).replace('.tar',''), 'tar', root_dir=firmware_dst.parent, base_dir=".")
    console.print(f"[green]Bundled app as {tar_path.name}[/green]")

    console.print(Panel(f"Build Complete\nArtifact: {tar_path}", style="bold blue"))

if __name__ == "__main__":
    main()