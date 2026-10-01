@echo off
cd /d "%~dp0"
title Smart Agriculture IoT Telemetry Dashboard
echo =====================================================================
echo    SMART AGRICULTURE SOIL MONITORING SYSTEM - WEB DASHBOARD
echo    Soil Nutrient and Moisture Monitoring System
echo =====================================================================
echo.
echo [1/2] Initializing local dashboard server on http://localhost:8080 ...
start /b "" "%USERPROFILE%\.platformio\penv\Scripts\python.exe" -m http.server 8080 --bind 127.0.0.1 >nul 2>&1
timeout /t 2 /nobreak >nul

echo [2/2] Opening dashboard in your default browser...
start http://localhost:8080/dashboard.html

echo.
echo =====================================================================
echo   DASHBOARD IS NOW RUNNING ON PORT 8080!
echo   1. Ensure ESP32 is plugged in via USB (COM3).
echo   2. Close the PlatformIO Serial Monitor if open (only 1 app can use COM3).
echo   3. Click 'Connect to Probe (COM3)' in the browser.
echo =====================================================================
echo.
echo Press any key to stop the dashboard server...
pause >nul
taskkill /f /im python.exe >nul 2>&1
