#Czech language.ini.cs file
#Version 1.09a by Spok (2003/07/31 - 09:32:48 CET :)
#Character set: "charset=Windows-1250"
 
# Use & for menu keys

#PROJECT_ID
[HEADER-Projects]
Title=Projekty
Project=Projekt
Account=Úèet
Total Credit=Celkovı kredit
Avg. Credit=Prùmìrnı kredit
Resource Share=Rozdìlení práce

#RESULT_ID
[HEADER-Work]
Title=Úkoly
Project=Projekt
Application=Aplikace
Name=Jméno
CPU time=Procesorovı èas
Progress=Prùbìh
To Completion=Dokonèí za
Status=Stav

#XFER_ID
[HEADER-Transfers]
Title=Pøenos
Project=Projekt
File=Soubor
Progress=Prùbìh
Size=Velikost
Time=Èas
Direction=Smìr
Speed=Rychlost
Status=Stav
Type=Typ

#MESSAGE_ID
[HEADER-Messages]
Title=Zprávy
Project=Projekt
Time=Èas
Message=Zpráva

#USAGE_ID
[HEADER-Disk]
Title=Disk
#Free space: not available for use=Volné místo: Nevyuitelné
#Free space: available for use=Volné místo: Vyuitelné
#Used space: other than BOINC=Obsazené místo: Mimo BOINC
#Used space:=Obsazené místo:
Free space=Volné místo
Used space: non-BOINC=Obsazené místo: mimo BOINC
Used space: BOINC=Obsazené místo: BOINC

#miscellaneous text
[HEADER-MISC]
New=Novı
Running=Probíhající
Ready to run=Pøipravenı
Computation done=Vıpoèet ukonèen
Results uploaded=Odesláno
Acknowledged=Potvrzeno
Error: invalid state=Chyba: Neplatnı stav
Completed=Dokonèeno
Upload=Odesílá se
Download=Pøijímá se
Retry in=Znovu zkusím za
Upload failed=Chyba pøi odesílání
Download failed=Chyba pøi stahování
Uploading=Odesílá se (*)
Downloading=Pøijímá se (*)


#menu items
# NOTE: add an & (ampersand) to the letter to be used as mnemonic
#       i.e. Show Graphics=Show &Graphics
#                               ^^ the "G" will trigger the menu item
#       you can compare it with a saved language.ini.XX file
[MENU-File]
Title=&Soubor
Show Graphics=Zobraz &Grafiku
Force run=Spus &Vıpoèet
Run based on preferences=Poèítej podle &Nastavení
Pause=&Pauza
Run Benchmarks=Spus &Testy
Hide=&Skryj
Exit=&Konec
Suspend=Pozastavit (*)
Resume=Obnovit (*)


[MENU-Settings]
Title=&Nastavení
Login to Project...=&Pøihlásit se k projektu...
Proxy Server...=Proxy &Server...

[MENU-Help]
Title=&Pomoc
About...=&O programu...

[MENU-StatusIcon]
Suspend=Pozastavit
Resume=Obnovit
Exit=Konec

[MENU-Project]
Relogin...=Znovu pøihlásit...
Get preferences=Získej nastavení
Retry now=Zkus teï
Detach...=Opustit projekt...
Reset project...=Restartovat projekt...
Quit Project...=Ukonèit projekt...

[MENU-Work]
Show Graphics=Zobraz &grafiku

[MENU-Transfers]
Retry now=Zkus teï

[MENU-Messages]
Copy to clipboard=Zkopíruj do schránky

[DIALOG-LOGIN]
Title=Pøihlásit se k projektu
URL:=URL:
Account Key:=Klíè (Account Key):
OK=OK
Cancel=Zrušit
The URL for the website of the project.=URL projektu.
The authorization code recieved in your confirmation email.=Autorizaèní klíè
obdrenı v potvrzovacím e-mailu.

[DIALOG-QUIT]
Title=Ukonèit
URL:=URL:
Account Key:=Klíè (Account Key):
OK=OK
Cancel=Zrušit
Select the project you wish to quit.=Vyberte projekt, kterı chcete ukonèit.

[DIALOG-CONNECT]
Title=Pøipojit
BOINC needs to connect to the network.  May it do so now?=BOINC vyaduje
pøipojení na internet. Pøipojit?
Don't ask this again (connect automatically)=Pøíštì se nedotazovat
(pøipojovat automaticky).
OK=OK
Cancel=Zrušit

[DIALOG-ABOUT]
Title=BOINC - Beta verze
Berkeley Open Infrastructure for Network Computing=Berkeley Open
Infrastructure for Network Computing
Open Beta=Open Beta
OK=OK

[DIALOG-PROXY]
Title=Nastavení proxy serveru
Some organizations use an "HTTP proxy" or a "SOCKS proxy" (or both) for
increased security.  If you need to use a proxy, fill in the information
below.  If you need help, ask your System Administrator or Internet Service
Provider.=Nìkteré organizace pouívají HTTP nebo SOCKS proxy server
(pøípadnì oba), kvùli zvıšení bezpeènosti. Pokud se pøipojujete pøes proxy
server, vyplòte tento formuláø. Potøebujete-li pomoc, kontaktujte vašeho
správce sítì nebo vašeho poskytovatele pøipojení.
HTTP Proxy=HTTP proxy server
Connect via HTTP Proxy Server=Pøipojit pøes HTTP proxy server
http://=http://
Port Number:=Èíslo portu:
SOCKS Proxy=SOCKS proxy server
Connect via SOCKS Proxy Server=Pøipojit pøes SOCKS proxy server
#SOCKS Host:=Jméno/IP-adresa SOCKS serveru:
SOCKS Host:=Jméno/IP:
Port Number:=Èíslo portu:
Leave these blank if not needed=Nevyplòujte, pokud není vyadováno
SOCKS User Name:=Uivatelské jméno:
SOCKS Password:=Heslo:
OK=OK
Cancel=Zrušit
