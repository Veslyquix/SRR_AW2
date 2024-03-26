@echo off

@rem USAGE: "MAKE HACK_full.cmd" [quick]
@rem If first argument is "quick", then this will not update text, tables, maps, or generate a patch
@rem "MACK HACK_quick.cmd" simply calls this but with the quick argument, for convenience

@rem defining buildfile config

set "source_rom=%~dp0AW2.gba"

set "main_event=%~dp0Installer.event"

set "target_rom=%~dp0AW2srr.gba"
set "target_ups=%~dp0AW2srr.ups"
set "target_sym=%~dp0AW2srr.sym"

@rem defining tools

set "c2ea=%~dp0Tools\C2EA\c2ea"
set "textprocess=%~dp0Tools\TextProcess\text-process-classic"
set "ups=%~dp0Tools\ups\ups"
set "parsefile=%~dp0EventAssembler\Tools\ParseFileUTF8.exe"
set "tmx2ea=%~dp0Tools\tmx2ea\tmx2ea"
set symcombo=%~dp0Tools\sym\SymCombo.exe

@rem set %~dp0 into a variable because batch is stupid and messes with it when using conditionals?

set "base_dir=%~dp0"

@rem do the actual building

echo Copying ROM

copy "%source_rom%" "%target_rom%"

echo:
echo Assembling

cd "%base_dir%EventAssembler"
ColorzCore A AW2 "-output:%target_rom%" "-input:%main_event%" --nocash-sym


  echo:
  echo Generating patch

  cd "%base_dir%"
  "%ups%" diff -b "%source_rom%" -m "%target_rom%" -o "%target_ups%"


echo:
echo Generating sym file

echo: | ( "%symcombo%" "%target_sym%" "%target_sym%" "AW2.sym" )

echo:
echo Done!

pause
