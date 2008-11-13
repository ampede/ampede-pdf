tell application "ToolServer"
	set sc1 to "MrC 'Macintosh HD:Users:bizman:Ampede:AmpedeFCP.c' -o 'Macintosh HD:Users:bizman:Build:AmpedeFCP.build:AmpedeFCP.c.x' -sym off -d TARGET_API_MAC_CARBON=1"
	
	set sc2 to "PPCLink -o 'Macintosh HD:Users:bizman:Build:AmpedeFCP' 'Macintosh HD:Users:bizman:Build:AmpedeFCP.build:AmpedeFCP.c.x' 'Macintosh HD:Applications (Mac OS 9):MPW-GM:Interfaces&Libraries:Libraries:SharedLibraries:CarbonLib' 'Macintosh HD:Applications (Mac OS 9):MPW-GM:Interfaces&Libraries:Libraries:SharedLibraries:StdCLib' 'Macintosh HD:Applications (Mac OS 9):MPW-GM:Interfaces&Libraries:Libraries:PPCLibraries:StdCRuntime.o' 'Macintosh HD:Applications (Mac OS 9):MPW-GM:Interfaces&Libraries:Libraries:PPCLibraries:PPCCRuntime.o' 'Macintosh HD:Applications (Mac OS 9):MPW-GM:Interfaces&Libraries:Libraries:PPCLibraries:PPCToolLibs.o' -sym off -mf -d -t 'eFKT' -c 'FXTC' -xm s -m main -export main"
	
	set sc3 to "Rez 'Macintosh HD:Users:bizman:Ampede:AmpedeFCPPiPL.r' -append -o 'Macintosh HD:Users:bizman:Build:AmpedeFCP'"
	
	DoScript sc1
	DoScript sc2
	DoScript sc3
end tell