REM Change the current directory to the directory in which bison.bat is.
REM This way bison.exe can find bison.simple.

cd %~dp0
bison.exe -o %1 %2
flex.exe -o%3 %4

REM    aq\parser\sql92_grm_tab.cpp    aq\parser\SQL92_Grm.y    aq\parser\lex.yy.cpp    aq\parser\SQL92_Tok.l
