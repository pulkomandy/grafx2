' VBScript to retrieve GIT "revision" and generate version.c
' Copyright 2018 Thomas Bernard
Set WshShell = CreateObject("WScript.Shell")
Set FSO = CreateObject("Scripting.FileSystemObject")
versionfile = "..\..\src\version.c"

On Error Resume Next

Function GetRevision()
	GetRevision = "unknown"
	Err.Clear
	Set oExec = WshShell.Exec("git rev-list --count 1af8c74f53110e349d8f0d19b14599281913f71f..")
	If Err.Number = 0 Then
		' Wait till the application is finished ...
		Do While oExec.Status = 0
		Loop
		If oExec.ExitCode = 0 Then
			GetRevision = oExec.StdOut.ReadLine()
		End If
	End If
End Function

Function GetBranch()
	GetBranch = "unknown"
	Err.Clear
	Set oExec = WshShell.Exec("git rev-parse --abbrev-ref HEAD")
	If Err.Number = 0 Then
		' Wait till the application is finished ...
		Do While oExec.Status = 0
		Loop
		If oExec.ExitCode = 0 Then
			GetBranch = oExec.StdOut.ReadLine()
		End If
	End If
End Function

GIT_REVISION = GetRevision()
GIT_BRANCH = GetBranch()
'Wscript.Echo "GIT_REVISION=" & GIT_REVISION & Chr(13) & Chr(10) & "GIT_BRANCH=" & GIT_BRANCH
revision = GIT_REVISION
If GIT_BRANCH <> "unknown" And GIT_BRANCH <> "master" Then
	revision = revision & "-" & GIT_BRANCH
End If
'Wscript.Echo revision

NeedWrite = True
Err.Clear
Set f = FSO.OpenTextFile(versionfile, 1, False) ' 1 = Read
If Err.Number = 0 Then
	line = f.ReadLine
	i = InStr(line, Chr(34)) + 1
	j = InStr(i, line, Chr(34))
	oldrevision = Mid(line, i, j - i)
	If revision = oldrevision Then
		NeedWrite = False
	End If
	f.Close
End If

If NeedWrite Then
	Set f = FSO.OpenTextFile(versionfile, 2, True) ' 2 = Write
	f.WriteLine "char SVN_revision[]=" & Chr(34) & revision & Chr(34) & ";"
	f.Close
End If