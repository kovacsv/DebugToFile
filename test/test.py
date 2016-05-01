import os
import subprocess

def WriteTitle (title):
	print '--- ' + title + ' ---'

def GetFileLines (fileName):
	file = open (resultFilePath)
	lines = file.readlines ()
	file.close ()
	return lines

def GetCommandOutput (command):
	process = subprocess.Popen (command, stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell = True)
	out, err = process.communicate ()
	return process.returncode, out, err
	
currentPath = os.path.dirname (os.path.abspath (__file__))
os.chdir (currentPath)

debugToFilePath = os.path.join ('..', 'solution', 'x64', 'Debug', 'DebugToFile.exe')
exampleProcessPath = os.path.join ('..', 'solution', 'x64', 'Debug', 'ExampleProcess.exe')
resultFilePath = 'test.txt'
success = True

WriteTitle ('Run process standalone')
ret, out, err = GetCommandOutput (exampleProcessPath)
print out

WriteTitle ('Run DebugToFile without parameters')
ret, out, err = GetCommandOutput (debugToFilePath)
print out
if out != 'Usage: DebugToFile.exe [DebugLogFileName] [ApplicationName] <ApplicationArguments>\r\n':
	success = False

WriteTitle ('Run DebugToFile with invalid process')
ret, out, err = GetCommandOutput (debugToFilePath + ' NotExisting.exe ' + resultFilePath + ' NotExisting.exe')
print out
if out != 'Error: Failed to start application\r\n':
	success = False

WriteTitle ('Run process with DebugToFile')
ret, out, err = GetCommandOutput (debugToFilePath + ' ' + resultFilePath + ' ' + exampleProcessPath)
print out

lines = GetFileLines (resultFilePath)
if lines[0] != exampleProcessPath + '\n':
	success = False
for i in range (1, len (lines)):
	if lines[i] != str (i - 1) + '\n':
		success = False
os.remove (resultFilePath)

WriteTitle ('Run process with DebugToFile with parameters')
ret, out, err = GetCommandOutput (debugToFilePath + ' ' + resultFilePath + ' ' + exampleProcessPath + ' -a -b -c')
print out
lines = GetFileLines (resultFilePath)
if lines[0] != exampleProcessPath + '\n':
	success = False
if lines[1] != '-a\n':
	success = False
if lines[2] != '-b\n':
	success = False
if lines[3] != '-c\n':
	success = False
for i in range (4, len (lines)):
	if lines[i] != str (i - 4) + '\n':
		success = False
os.remove (resultFilePath)

WriteTitle ('Result')
if success:
	print 'OK'
else:
	print 'FAILED'
