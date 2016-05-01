import os

def WriteTitle (title):
	print '--- ' + title + ' ---'

def GetFileLines (fileName):
	file = open (resultFilePath)
	lines = file.readlines ()
	file.close ()
	return lines

currentPath = os.path.dirname (os.path.abspath (__file__))
os.chdir (currentPath)

debugToFilePath = os.path.join ('..', 'solution', 'x64', 'Debug', 'DebugToFile.exe')
exampleProcessPath = os.path.join ('..', 'solution', 'x64', 'Debug', 'ExampleProcess.exe')
resultFilePath = 'test.txt'

WriteTitle ('Run process standalone')
os.system (exampleProcessPath)

WriteTitle ('Run process with DebugToFile')
os.system (debugToFilePath + ' ' + resultFilePath + ' ' + exampleProcessPath)
lines = GetFileLines (resultFilePath)
success = True
if lines[0] != exampleProcessPath + '\n':
	success = False
for i in range (1, len (lines)):
	if lines[i] != str (i - 1) + '\n':
		success = False
os.remove (resultFilePath)

WriteTitle ('Run process with DebugToFile with parameters')
os.system (debugToFilePath + ' ' + resultFilePath + ' ' + exampleProcessPath + ' -a -b -c')
lines = GetFileLines (resultFilePath)
success = True
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
