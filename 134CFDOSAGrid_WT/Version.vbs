' VBScript
Set arArgs = WScript.Arguments
Set objFSO = CreateObject("Scripting.FileSystemObject")
Set WshShell = CreateObject("WScript.Shell")

Wscript.Echo "*** WebTracker DLL Version Renaming Script ***"
nPos = 0
' Make sure parameters are passed
If arArgs.Count > 0 Then    
	' Look for Debug flag
	If arArgs(nPos) = "Debug" Then
		bDebug = True
		nPos = 1 
	End If
	
	' Load Filename from arguments
	strFileName = arArgs(nPos)
	If bDebug Then
		Wscript.Echo "PRODUCT : " & strFileName
	End If
	
	' Get the Version Number from the file
	strVersion = objFSO.GetFileVersion(strFileName)
	If bDebug Then
		Wscript.Echo "VERSION : " & strVersion
	End If
	' Remove periods in version
	strVersion = Replace(strVersion, ".", "")
	
	' Change filename to include version number
	strCopyFileName = Replace(strFileName, ".", strVersion & ".")
	If bDebug Then
		Wscript.Echo "CREATED : " & strCopyFileName
	End If	
	
	' Copying file to Local Directory
	objFSO.CopyFile strFileName, strCopyFileName 
	If Err.Number <> 0 Then
		Wscript.Echo "Error copying " & strFileName & " to " & strCopyFileName
	End If
	' Copying file to HTML surf directory
	objFSO.CopyFile strCopyFileName, "..\HTML\surf\lib\" 
	If Err.Number <> 0 Then
		Wscript.Echo "Error copying " & strCopyFileName & " to ..\HTML\surf\lib\"
	End If
	If bDebug Then
		Wscript.Echo "RELEASE : ..\HTML\surf\lib\" & objFSO.GetFileName(strCopyFileName)
	End If
	
	' Create crc filename
	'strCRCFileName = Replace(strCopyFileName, ".dll", ".crc")
	'If bDebug Then
	'	Wscript.Echo "CRC FILENAME: " & strCRCFileName
	'End If
	
	' Create crc shell command 
	'strCommand = "crc.exe -f " & strCopyFileName 
	'If bDebug Then
	'	Wscript.Echo "COMMAND: " & strCommand
	'End If
	
	' Execute shell command (create actual CRC file)
	'Set oExec = WshShell.Exec(strCommand)
	'Do While oExec.Status = 0
    '   WScript.Sleep 100
	'Loop
	
	'strResult = oExec.StdOut.ReadLine
	'If bDebug Then
	'	WScript.Echo "CRC: " & strResult
	'End If
	
	' CRC File crc file
	'Set crcFile = objFSO.CreateTextFile(strCRCFileName)
	'crcFile.WriteLine strResult
	'crcFile.Close
	
	' Copying file to HTML surf directory
	'objFSO.CopyFile strCRCFileName, "..\HTML\surf\lib\" 	
	Wscript.Echo "*** WebTracker DLL Version Renaming Script Finished ***"
	Wscript.Echo " "
Else
	Wscript.Echo "Usage: Version.vbs [DEBUG] dll"
End If