[![License](https://img.shields.io/gitlab/license/xifil/t9-mod.svg)](https://gitlab.com/xifil/t9-mod/-/blob/master/LICENSE)
[![Open Issues](https://img.shields.io/gitlab/issues/open/xifil/t9-mod.svg)](https://gitlab.com/xifil/t9-mod/-/issues)
[![Discord](https://img.shields.io/discord/769966964030046298?color=%237289DA&label=members&logo=discord&logoColor=%23FFFFFF)](https://discord.gg/dPzJajt)

# t9-mod

<p align="center">
  <img src="assets/github/banner.png?raw=true" />
</p>

NOTE: You must legally own [Call of Duty®: Black Ops Cold War](https://store.steampowered.com/app/1985810/) to run this mod. Cracked/Pirated versions of the game are **NOT** supported.

## Compile from source

- Clone the Git repo. Do NOT download it as ZIP, that won't work.
  > You can run
  > ```
  > git clone https://github.com/xifil/t9-mod.git --recurse-submodules
  > ```
  > or
  > ```
  > git clone https://gitlab.com/xifil/t9-mod.git --recurse-submodules
  > ```
  > depending on what Git you would like to use, however you always need the `--recurse-submodules` flag.
- Update the submodules and run `premake5 vs2022` or simply use the delivered `generate.bat`.
- Build via solution file in `t9_vs2022.sln`.

<!--
### Premake arguments

| Argument                    | Description                                    |
|:----------------------------|:-----------------------------------------------|
| `--copy-to=PATH`            | Optional, copy the EXE to a custom folder after build, define the path here if wanted. |
| `--dev-build`               | Enable development builds of the client. |

<br/>-->

## Download from Actions

GitHub Actions automatically builds the DLL file on each 
commit, you can find the latest build of t9-mod 
[here](https://xifil.github.io/t9-redirect).

## Disclaimer

This software has been created purely for the purposes of
academic research. It is not intended to be used to attack
other systems. Project maintainers are not responsible or
liable for misuse of the software. Use responsibly.

## Usage

t9-mod supports these versions of Black Ops Cold War:  
- 1.34.0.15931218
- 1.34.1.15931218

No, you cannot play online with this - the local play doesn't work with other players.

The same `discord_game_sdk.dll` can be used for both versions as it auto-detects the version based on the executable and calculates the signatures based on that.

## Credits

- [t6-mod](https://gitlab.com/xifil/t6-mod) - codebase
- [Defcon](https://github.com/ProjectDonetsk/Defcon) - documented code used throughout the project
