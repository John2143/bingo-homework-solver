@echo off
setLocal EnableDelayedExpansion
for /f "tokens=* delims= " %%a in (debug.txt) do (
set var9=!var8!
set var8=!var7!
set var7=!var6!
set var6=!var5!
set var5=!var4!
set var4=!var3!
set var3=!var2!
set var2=!var1!
set var1=!var!
set var=%%a
)
echo !var9!
echo !var8!
echo !var7!
echo !var6!
echo !var5!
echo !var4!
echo !var3!
echo !var2!
echo !var1!
echo !var!