#!/usr/bin/env python3
"""Build PocketMage OS firmware."""

import os
import sys
import subprocess
import shutil
from pathlib import Path
from rich.console import Console
from rich.panel import Panel
from rich.progress import Progress

def main():
    console = Console()
    workspace_root = Path(__file__).parent.parent
    os.chdir(workspace_root)
    os_dir = workspace_root / "PocketMage_V3"
    build_dir = os_dir / ".pio" / "build"

    console.print(Panel("PocketMage OS Build", style="bold green"))

    console.print("[bold]Installing PlatformIO...[/bold]")
    result = subprocess.run(["python3", "-m", "pip", "install", "--upgrade", "platformio"])
    if result.returncode != 0:
        console.print("[red]PlatformIO installation failed[/red]")
        sys.exit(1)
    console.print("[green]PlatformIO installed[/green]")

    console.print("[bold]Building with PlatformIO...[/bold]")
    result = subprocess.run(["pio", "run"], cwd=os_dir)
    if result.returncode != 0:
        console.print("[red]PlatformIO build failed[/red]")
        sys.exit(1)
    console.print("[green]Build completed[/green]")

    firmware_files = list(build_dir.glob("*/firmware.bin"))
    bootloader_files = list(build_dir.glob("*/bootloader.bin"))
    partition_files = list(build_dir.glob("*/partitions.bin"))
    if not firmware_files or not bootloader_files or not partition_files:
        console.print("[red]firmware.bin, bootloader.bin, or partitions.bin not found in build directory[/red]")
        sys.exit(1)

    firmware_src = firmware_files[0]
    bootloader_src = bootloader_files[0]
    partition_src = partition_files[0]

    firmware_dst = os_dir / "os_build" / "firmware.bin"
    bootloader_dst = os_dir / "os_build" / "bootloader.bin"
    partition_dst = os_dir / "os_build" / "partitions.bin"

    firmware_dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(firmware_src, firmware_dst)
    shutil.copy2(bootloader_src, bootloader_dst)
    shutil.copy2(partition_src, partition_dst)
    console.print("[green]Copied OS firmware[/green]")

    console.print(Panel(f"Build Complete\nArtifacts:\n {firmware_dst}\n {bootloader_dst}\n {partition_dst}", style="bold blue"))

if __name__ == "__main__":
    main()
