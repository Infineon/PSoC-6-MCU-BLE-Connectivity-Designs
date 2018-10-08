@rem Usage:
@rem Call post_build_core1.bat <tool> <output_dir> <project_short_name> <compiler>
@rem E.g. in PSoC Creator 4.2:
@rem      post_build_core1.bat creator ${OutputDir} ${ProjectShortName}

@echo ------------------------------------------
@echo   Post-build commands for Cortex-M4 core
@echo ------------------------------------------

@rem Set proper path to your PDL 3.x and above installation
@set PDL_PATH="C:\Program Files (x86)\Cypress\PDL\3.0.1"

@set CY_MCU_ELF_TOOL=%PDL_PATH%"\tools\win\elf\cymcuelftool.exe"

@set IDE=%1

@if "%IDE%" == "creator" (
    @set OUTPUT_DIR=%2
    @set PRJ_NAME=%3
    @set ELF_EXT=.elf
    @set COMPILER=%4
)

@if "%IDE%" == "uvision" (
    @set OUTPUT_DIR=%2
    @set PRJ_NAME=%3
    @set ELF_EXT=.axf
    @set COMPILER=%4
)

@if "%IDE%" == "iar" (
    @set OUTPUT_DIR=%2
    @set PRJ_NAME=%3
    @set ELF_EXT=.out
    @set COMPILER=%4
)

@if "%IDE%" == "eclipse" (
    @set OUTPUT_DIR=%2
    @set PRJ_NAME=%3
    @set ELF_EXT=.out
    @set COMPILER=%4
)

@rem Populates the signature field of the application with a CRC
%CY_MCU_ELF_TOOL% -S %OUTPUT_DIR%\%PRJ_NAME%%ELF_EXT% CRC --output %OUTPUT_DIR%\%PRJ_NAME%%ELF_EXT% --hex %OUTPUT_DIR%\%PRJ_NAME%_premerge.hex

@rem Generates assembly code sharing file for App2 Core1
%CY_MCU_ELF_TOOL% --codeshare %OUTPUT_DIR%\%PRJ_NAME%%ELF_EXT% BLE_Symbols.txt %COMPILER% --output ..\Bootloader_BLE_Upgradable_Stack_App2.cydsn\BLE_core1_shared.s

@rem Creates the bootloadable file
%CY_MCU_ELF_TOOL% -P %OUTPUT_DIR%\%PRJ_NAME%%ELF_EXT% --output %OUTPUT_DIR%\%PRJ_NAME%.cyacd2

@rem Merges App0 and App1 into a single hex file for easier programming
%CY_MCU_ELF_TOOL% -M %OUTPUT_DIR%\%PRJ_NAME%%ELF_EXT% ..\App0%ELF_EXT% --output %OUTPUT_DIR%\%PRJ_NAME%_merged%ELF_EXT% --hex %OUTPUT_DIR%\%PRJ_NAME%.hex

@rem Copies the resulting linker file to a more accessible location for later merging
COPY "%OUTPUT_DIR%\%PRJ_NAME%%ELF_EXT%" "..\App1%ELF_EXT%"