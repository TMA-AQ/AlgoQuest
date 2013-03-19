REM Change the current directory to the directory in which bison.bat is.
REM This way bison.exe can find bison.simple.

cd %~dp0
bison.exe -o %1 %2
flex.exe -o%3 %4

REM SQLParser\sql92_grm_tab.c SQLParser\SQL92_Grm.y SQLParser\lex.yy.c SQLParser\SQL92_Tok.l
