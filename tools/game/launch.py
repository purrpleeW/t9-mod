import argparse
import json
import os
import shutil
import subprocess
import tkinter
import tkinter.filedialog
import tkinter.messagebox

wnd_root = tkinter.Tk()
wnd_root.withdraw()

supported_game_versions = [
	"1.34.0.15931218",
	"1.34.1.15931218"
]
mod_dll_name = "discord_game_sdk.dll"

directories = {}
directories_path = os.path.join(os.path.dirname(__file__), "directories.json")

if os.path.exists(directories_path):
	with open(directories_path, "r") as f:
		directories = json.load(f)

for version in supported_game_versions:
	if version not in directories:
		directories[version] = ""

def save_directories():
	global directories
	global directories_path
	with open(directories_path, "w") as f:
		json.dump(directories, f)

argument_parser = argparse.ArgumentParser(description="Call of Duty: Black Ops Cold War Launcher")

argument_parser.add_argument("--version", type=str, choices=supported_game_versions, help="Version of Black Ops Cold War to launch",
	default="1.34.0.15931218")
argument_parser.add_argument("--custom-exe", type=str, help="Choose a custom Black Ops Cold War executable", default="BlackOpsColdWar")

arguments = argument_parser.parse_args()

arg_version = str(arguments.version)
arg_custom_exe = str(arguments.custom_exe)

if not arg_custom_exe.endswith(".exe"):
	arg_custom_exe += ".exe"

selected_directory = directories[arguments.version]
if len(selected_directory) < 1 or not os.path.isdir(selected_directory):
	selected_directory = tkinter.filedialog.askdirectory(title=f"Find game directory of Black Ops Cold War v{arguments.version}")

if not selected_directory:
	tkinter.messagebox.showinfo("t9-mod", "No directory chosen, cancelling.")
	exit()

directories[arguments.version] = selected_directory
save_directories()

arg_custom_exe_absolute = os.path.join(selected_directory, arg_custom_exe)
if not os.path.isfile(arg_custom_exe_absolute):
	tkinter.messagebox.showinfo("t9-mod", f"Couldn't find executable \"{arg_custom_exe_absolute}\".")
	exit()

t9_mod_dll = os.path.join(os.path.dirname(__file__), "..", "..", "build", "t9_vs2022", "x64", "client", mod_dll_name)
try:
	shutil.copy(t9_mod_dll, os.path.join(selected_directory, mod_dll_name))
except FileNotFoundError:
	tkinter.messagebox.showinfo("t9-mod", f"Couldn't find new {mod_dll_name}, not updating.")
except Exception as e:
	tkinter.messagebox.showinfo("t9-mod", f"Caught an unknown exception: {e}")

os.chdir(selected_directory)
subprocess.Popen([ arg_custom_exe ])
