dir /B *txt > _all_txt_files
 
 
@echo off
for /f "tokens=*" %%a in (_all_txt_files) do (
  echo %%a;
dot.exe -Tpng %%a > %%a.png
)
