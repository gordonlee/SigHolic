@echo off 
setlocal enableDelayedExpansion 

set file_list=D:\GitHub\SigHolic\checker_list.txt
FOR /F %%i IN (D:\GitHub\SigHolic\checker_list.txt) DO (
	ECHO %%i
	CALL python.exe D:\GitHub\SigHolic\script\cpplint.py %%i
)