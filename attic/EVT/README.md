# General Tooling Notes For Using Official EVT With GCC but without Moun River

This shows how to build the CH32V003 demo binaries without the horrifying MounRiver Studio.

* Product page: https://www.wch.cn/products/CH32V003.html
* or English: http://www.wch-ic.com/products/CH32V003.html
* It has, for the English datasheet http://www.wch-ic.com/downloads/file/359.html "CH32V003DS0.PDF"
* It also has the TRM http://www.wch-ic.com/downloads/CH32V003RM_PDF.html in English
* Sadly, the EVT is CN only. https://www.wch.cn/downloads/CH32V003EVT_ZIP.html - It contains schematics for the devboard, evaluation board reference (instructions) and the rest of the dev system, minus the MounRiver Studio (MRS).
* Software from WCH for WCH-Link: https://www.wch-ic.com/products/WCH-Link.html
* Get https://www.wch.cn/downloads/WCH-LinkUtility_ZIP.html

## Steps to run an EVT sample

1. Install WCHLinkDrv_WHQL_S.exe from Wch-LinkUtil/Drv_Link
2. Copy contents of EVT (CH32V003EVT.ZIP) "EXAM" Folder in to the EVT folder.
3. On WSL or Debian based OSes `apt-get install build-essential libnewlib-dev gcc-riscv64-unknown-elf`
4. cd into `EVT` on WSL or Linux
5. Type `make`
6. Open Wch-LinkUtil
7. Select series "CH32V00X" from pull-down.
8. Open the produced .hex file.
9. Target->Program (F10)
10. Target->Reset (F12)

Profit.


