@cd /D %~dp0
:: Run bash and build the project
:: Arguments are:
:: full path to build tools directory (unix path, not windows)
:: full path to project directory (unix path, not windows)
:: ...

set BUILD_TOOLS_DIRECTORY=%1
set PROJECT_DIRECTORY=%2

bash.exe %BUILD_TOOLS_DIRECTORY% %PROJECT_DIRECTORY%
