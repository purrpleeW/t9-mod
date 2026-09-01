@echo off
git remote set-url origin https://github.com/xifil/t9-mod.git
git fetch && git pull
git remote set-url origin https://gitlab.com/xifil/t9-mod.git
git push
