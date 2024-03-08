
===============================================================================
      How to install the version 6.71.00.00 onto x86 operating system
===============================================================================

1. Extract all files from the zip-file to any directory on your PC.

2. Go to x86 folder

3. Rename the following files:
   ACS.EStop.$exe to ACS.EStop.exe
   ACSC.$H to ACSC.H
   ACSCL_x86.$DLL to ACSCL_x86.DLL
   ACSCL_x86.$LIB to ACSCL_x86.LIB
   ACSCSRV.$EXE to ACSCSRV.EXE
   SPiiPlusCOM660_x86.$dll to SPiiPlusCOM660_x86.dll
   ACS.SPiiPlusNET.$dll to ACS.SPiiPlusNET.dll

4. Copy ACSCSRV.EXE and ACS.EStop.exe to the "User Mode Driver"
   directory (default C:\Program Files\ACS Motion Control\SPiiPlus ADK Suite v3.02
   \User Mode Driver) 
   directory and overwrite the previous file with the same name.

5. Copy ACSCL_x86.DLL to Windows system directory (C:\WINDOWS\SYSTEM32)

6. Copy SPiiPlusCOM660_x86.dll and ACS.SPiiPlusNET.dll to the "CommonFiles"
   directory (default C:\Program Files\ACS Motion Control\CommonFiles)
   and overwrite the previous files with the same name.

7. Copy ACSC.H and ACSCL_x86.LIB files to your C\C++ projects directory 
   and overwrite the previous files with the same name. If ACSCL.LIB file
   was previously included in project, it should be removed and the file 
   should be deleted.

It is required to rebuild your application to apply the changes.


===============================================================================
      How to install the version 6.71.00.00 onto x64 operating system
===============================================================================

1. Extract all files from the zip-file to any directory on your PC.

2. Go to x86 folder

3. Rename the following files:
   SPiiPlusCOM660_x86.$dll to SPiiPlusCOM660_x86.dll

4. Go to x64 folder

5. Rename the following files:
   ACS.EStop.$exe to ACS.EStop.exe
   ACSC.$H to ACSC.H
   ACSCL_x64.$DLL to ACSCL_x64.DLL
   ACSCL_x64.$LIB to ACSCL_x64.LIB
   ACSCSRV.$EXE to ACSCSRV.EXE
   SPiiPlusCOM660_x64.$dll to SPiiPlusCOM660_x64.dll

6. Copy ACSCSRV.EXE and ACS.EStop.exe to the "User Mode Driver"
   directory (default C:\Program Files (x86)\ACS Motion Control
   \SPiiPlus ADK Suite v3.02\User Mode Driver) 
   directory and overwrite the previous file with the same name.

7. Copy ACSCL_x86.DLL to 32-BIT Windows system directory (C:\WINDOWS\SYSWOW64)

8. Copy ACSCL_x64.DLL to 64-BIT Windows system directory (C:\WINDOWS\SYSTEM32)

9. Copy SPiiPlusCOM660_x86.dll, SPiiPlusCOM660_x64.dll, and ACS.SPiiPlusNET.dll
   to the "CommonFiles" directory
   (default C:\Program Files (x86)\ACS Motion Control\CommonFiles)
   and overwrite the previous files with the same name.
 
10.Copy ACSC.H and ACSCL_x86.LIB files to your C\C++ projects directory 
   and overwrite the previous files with the same name.

It is required to rebuild your application to apply the changes.