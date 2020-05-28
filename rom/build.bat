@echo off
set build_date=%date:~10,4%%date:~4,2%%date:~7,2%%time:~0,2%%time:~3,2%

armips -strequ build_date %build_date% -sym n00brom.sym -temp n00brom.lst n00brom.asm

copy /B romhead.bin+ramprog.bin n00brom.rom