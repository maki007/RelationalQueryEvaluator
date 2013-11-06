dir /B *txt > all_txt_files
 
@echo off
for /f "tokens=*" %%a in (all_txt_files) do (
  echo %%a;
dot.exe -Tpng %%a > %%a.png
)
