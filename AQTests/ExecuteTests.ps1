# Read Param
Param([Boolean]$help,
	  [String]$settings, 
	  [String]$testsFolder,
	  [String]$databasesFolder,
	  [String]$aq_query_resolver,
	  [String]$aqengine,
	  [String]$cutInCol,
	  [String]$loader,
	  [String]$logFile,
	  [String]$checkFile,
	  [String]$skipFile,
	  [Boolean]$outputToNotepad)

# Global Settings default values
$requestsFolder = "Requests"
$resultsFolder = "Results"
$expectedFolder = "Expected"
$requestAnswerFolder = "calculus\test\"
$requestFile = "request.txt"
$resultFilesStart = @("New_Request", "Answer", "aq_query_resolver.log", $requestFile)
$deleteFiles = @("Answer_log.txt")
$logName = "./Logs/TestLog_"
$logExt = ".txt"

$iniFile = "aq_query_resolver.ini"
$rootFolderKey = "root-folder"
$tmpFolderKey = "tmp-folder"
$fieldSeparatorKey = "field-separator"
$aqEngineKey = "aq-engine"
$loaderKey = "aq-loader"

$answerFileName = "Answer.txt"

$success = 0
$warnings = 0
$errors = 0
$aqEngineErrors = 0
$unchecked = 0
$skipped = 0
$noDBErrors = 0

Function LoadRequests($file)
{
	$content = ""
	if ($file)
	{
		foreach ($line in Get-Content $file)
		{
			$line = $line.Trim()
			if (!$line -or $line.SubString(0, 1).CompareTo("#") -eq 0) { continue }
			else { $content += "[" + $line + "]"}
		}
	}
	return $content
}

#******************************************************************************
Function CheckResult($logFile, $testName, $fileName, $folderResult, $execTime, [ref] $errors, [ref] $unchecked, [ref] $success, [ref] $aqEngineErrors)
{
    $fileExpected = Join-Path $folderResult (Join-Path $expectedFolder $fileName)
    $fileDest = Join-Path $folderResult (Join-Path $resultsFolder $fileName)
    
	$AQEngineFileDest = $fileName -replace ".txt", "_Before.txt"
    $AQEngineFileDestExpected = Join-Path $folderResult (Join-Path $expectedFolder $AQEngineFileDest)
    $AQEngineFileDestResult = Join-Path $folderResult (Join-Path $resultsFolder $AQEngineFileDest)
    
	$folderResult = Join-Path $folderResult $resultsFolder
	if ( !(Test-Path $folderResult) )
	{
		echo "create path $folderResult"
		New-Item $folderResult -Type Directory 
	}
	
    if( Test-Path $fileExpected )
    {
		# check final answer
        [string]$content1 = Get-Content $fileDest
        [string]$content2 = Get-Content $fileExpected
        if( $content1.CompareTo($content2) -ne 0 )
        {
			# check aq engine answer
			if (Test-Path $AQEngineFileDestExpected)
			{
				if (Test-Path $AQEngineFileDestResult)
				{
					[string]$aq_engine_answer_content1 = Get-Content $AQEngineFileDestResult
					[string]$aq_engine_answer_content2 = Get-Content $AQEngineFileDestExpected
					if ( $aq_engine_answer_content1.CompareTo($aq_engine_answer_content2) -ne 0 )
					{
						Write-Warning $($testName + " AQEngine FAILED")
						Add-Content $logFile ($testName + " AQEngine ERROR: actual response does not match expected response")
						++$aqEngineErrors.Value
					}
					else
					{
						Write-Warning $($testName + " aq_query_resolver FAILED")
						Add-Content $logFile ($testName + " aq_query_resolver ERROR: actual response does not match expected response")
					}
				}
				else
				{
					Write-Warning $($testName + " AQEngine FAILED")
					Add-Content $logFile ($testName + " AQEngine ERROR: No file found")
					++$aqEngineErrors.Value
				}
			}
			else
			{
				Write-Warning $($testName + " aq_query_resolver FAILED (BUT AQENGINE Not Checked)")
				Add-Content $logFile ($testName + " aq_query_resolver ERROR: actual response does not match expected response (BUT AQENGINE Not Checked)")
            }
			++$errors.Value
        }
		else
		{
			++$success.Value
			Write-Output $($testName + " SUCCESSFULLY CHECKED in " + $execTime + " sec")
			Add-Content $logFile ($testName + ";OK")
		}
    }
    else
    {
		Write-Output $($testName + " UNCHECKED (" + $fileExpected + ")")
        Add-Content $logFile ($testName + ";UNCHECKED")
        ++$unchecked.Value
    }
}

#******************************************************************************
Function CreateIniFile($database, $absolute)
{
	if (! (Test-Path $iniFile) )
	{
		New-Item $iniFile -type file
	}
	else
	{
		Clear-Content $iniFile
	}
	
	Add-Content $iniFile ($rootFolderKey + " = " + (Join-Path $databasesFolder $database) + "\")
	Add-Content $iniFile ($tmpFolderKey + " = " + (Join-Path $databasesFolder $database) + "\data_orga\tmp\")
	Add-Content $iniFile ($fieldSeparatorKey + " = ;")
	Add-Content $iniFile ($aqEngineKey + " = `"" + $aqengine + "`"")
	Add-Content $iniFile ($loaderKey + " = `"" + $loader + "`"")
}

#******************************************************************************
Function GetSelectedFolders([ref] $requestsToCheck)
{
    $testFolders = @(dir $testsFolder | where {$_.PsIsContainer} | select -ExpandProperty Name)
    for( $idx = 0; $idx -lt $testFolders.length; ++$idx )
    {
    	$name = $testFolders[$idx]
    	Write-Host "[$idx] $name"
    }
    Write-Host "[Enter - All]"
    [string]$selection = Read-Host
    $selection = $selection.Trim()
    if( $selection -eq "" )
    {   
		$selectedFolders = $testFolders 
	}
    else
    { 
		$requestsToCheck = ""
		foreach ($id in $selection.Split(' '))
		{
			$selectionInt = [System.Int32]::Parse($id)
			$selectedFolders += @($testFolders[$selectionInt])
		}
    }
	
    return $selectedFolders
}

#******************************************************************************
Function DeleteExtraFiles($requestAnswerFolderFull)
{
    foreach($deleteFile in $deleteFiles)
    {
        $deleteFileFull = (Join-Path $requestAnswerFolderFull $deleteFile)
        if( Test-Path $deleteFileFull )
	    {
	        Remove-Item $deleteFileFull
	    }
    }
}

#******************************************************************************
Function DeletePreviousResults($testFolder)
{
	$previousFolder = Join-Path $testFolder $resultsFolder
	if (Test-Path $previousFolder)
	{
		foreach($ext in @("*.txt", "*.sql", "*.log", "*.ini"))
		{
			$previousAnswers = Join-Path $testFolder (Join-Path $resultsFolder $ext)
			if( Test-Path $previousAnswers )
			{
				Remove-Item $previousAnswers
			}
		}
	}
}

#******************************************************************************
#main

if ($help)
{
	Write-Host "Usage:"
	Write-Host "	-settings         			"
	Write-Host "	-databasesFolder  			"
	Write-Host "	-aq_query_resolver 			"
	Write-Host "	-aqengine         			"
	Write-Host "	-testsFolder      			"
	Write-Host "	-logFile          			"
	Write-Host "	-checkFile         			requests to check"
	Write-Host "	-skipFile	      			requests to skip"
	Write-Host "	-outputToNotepad  			"
	return
}

#
# Read ini settings file
if (!$settings) { $settings = "ExecuteTests.ini" }
if ($settings)
{
	foreach ($line in Get-Content $settings)
	{
		$line = $line.Trim()
		if (($line.CompareTo("") -eq 0) -or ($line.SubString(0, 1).CompareTo("#") -eq 0))
		{
			continue
		}
		$pos = $line.IndexOf("=")
		if ($pos -le 0) { continue ; }
		$el1 = $line.SubString(0, $pos)
		$el2 = $line.SubString($pos+1)
		$el1 = $el1.Trim()
		$el2 = $el2.Trim()
		if (!$databasesFolder 					-and ($el1.CompareTo("databasesFolder") 				-eq 0) ) { $databasesFolder 			= $el2 ; continue }
		if (!$aq_query_resolver      			-and ($el1.CompareTo("aq_query_resolver")      				-eq 0) ) { $aq_query_resolver      			= $el2 ; continue }
		if (!$aq_query_resolver_params      	-and ($el1.CompareTo("aq_query_resolver_params")	    -eq 0) ) { $aq_query_resolver_params    = $el2 ; continue }
		if (!$aqengine        					-and ($el1.CompareTo("aqengine")        				-eq 0) ) { $aqengine        			= $el2 ; continue }
		if (!$cutInCol        					-and ($el1.CompareTo("cutInCol")        				-eq 0) ) { $cutInCol        			= $el2 ; continue }
		if (!$loader        					-and ($el1.CompareTo("loader")	        				-eq 0) ) { $loader	        			= $el2 ; continue }
		if (!$testsFolder     					-and ($el1.CompareTo("testsFolder")     				-eq 0) ) { $testsFolder     			= $el2 ; continue }
		if (!$logFile         					-and ($el1.CompareTo("logFile")         				-eq 0) ) { $logFile         			= $el2 ; continue }
		if (!$checkFile        					-and ($el1.CompareTo("check")	        				-eq 0) ) { $checkFile       			= $el2 ; continue }
		if (!$skipFile	     					-and ($el1.CompareTo("skip")		     				-eq 0) ) { $skipFile     				= $el2 ; continue }
		if (!$outputToNotePad 					-and ($el1.CompareTo("outputToNotePad") 				-eq 0) ) 
		{ 
			if ($el2.CompareTo("1") -eq 0) { $outputToNotePad = $True }
			else { $outputToNotePad = $False }
			continue 
		}
	}
}

#
# Check Values are setted
if ((!$databasesFolder) -or
	(!$aq_query_resolver) -or
	(!$aqengine) -or
	(!$cutInCol) -or
	(!$loader) -or
	(!$testsFolder))
{
	Write-Host "Variables are not setted"
	return
}

Write-Host "Settings:"
Write-Host "========="
Write-Host "testsFolder:      		$testsFolder"
Write-Host "databasesFolder:  		$databasesFolder"
Write-Host "aq_query_resolver:		$aq_query_resolver"
Write-Host "aqengine:         		$aqengine"
Write-Host "logFile:          		$logFile"
Write-Host "onlyFile:         		$checkFile"
Write-Host "excludeFile:      		$skipFile"
Write-Host ""

Write-Host "Continue? [y/n]"
[string]$selection = Read-Host
$selection = $selection.Trim()
if( ! ($selection -eq "y" ) )
{
    return
}

#
# Get Request to exclude
$excludeRequests = LoadRequests $skipFile

#
# Get Requests to tests
$requestsToCheck = LoadRequests $checkFile

#
# Get databases
$databases = @(dir $databasesFolder | where {$_.PsIsContainer} | select -ExpandProperty Name)

Write-Host "Databases:"
Write-Host "=========="
Write-Host $databases
Write-Host ""

#
# Select folders
$selectedFolders = GetSelectedFolders ([ref] $requestsToCheck)

$date = Get-date
$smalldate = Get-Date -format "yyyy_MM_dd_HH_mm_ss"
if (!$logFile)
{
	$logFile = $logName +  $smalldate + $logExt
}

if( Test-Path $logFile )
{
	Clear-Content $logFile
}
Add-Content $logFile ("Start: $date")

foreach($testFolder in $selectedFolders)
{
	$testFolder = Join-Path $testsFolder $testFolder
	$folder = Join-Path $testFolder $requestsFolder
	$requests = @(dir $folder | where {!$_.PsIsContainer})
	$previousFolder = Join-Path $testFolder $resultsFolder
	if (!(Test-Path -path $previousFolder))
	{
		New-Item $previousFolder -ItemType directory
	}
    DeletePreviousResults $previousFolder
	foreach($request in $requests)
	{
		$requestFullName = Join-Path $folder $request.Name
	
		if ($excludeRequests -and ($excludeRequests.Contains($requestFullName) ) )
		{		
			$line = $requestFullName + " SKIPPED"
			# Write-Output $line
			# Add-Content $logFile $line
			++$skipped
			continue
		}
	
		if ($requestsToCheck -and (! ($requestsToCheck.Contains($requestFullName) ) ) )
		{		
			$line = $requestFullName + " SKIPPED"
			# Write-Output $line
			# Add-Content $logFile $line
			++$skipped
			continue
		}
	
		Write-Host ""
		# Write-Host $("processing " + $requestFullName)
		
		#
        #get test file
		$file = Get-Content $requestFullName
		
		#
        #check if first line is a valid database name
		$database = $file[0].Trim()
		if ($database.SubString(0, 2).CompareTo("--") -eq 0)
		{
			$database = $database.SubString(2)
			$database = $database.Trim()
		}
		if( !($databases -contains $database) )
		{ 
			Write-Warning $($requestFullName + ":ERROR invalid database '" + $database + "'")
			Add-Content $logFile ($requestFullName + " ERROR invalid database: " + $database)
		    ++$noDBErrors
			continue
		}
	        
		#write test to where aq_query_resolver reads it from
	    $requestAnswerFolderFull = Join-Path $databasesFolder (Join-Path $database $requestAnswerFolder)
		DeletePreviousResults $requestAnswerFolderFull
		$targetRequest = Join-Path $requestAnswerFolderFull $requestFile
		Set-Content -Encoding UTF8 $targetRequest $file[1 .. $file.length]
        
		CreateIniFile $database 1

		Write-Output $("check " + $testFolder + "\" + $request + " on db '" + $database + "'")
		
		
		#
		# run aq_query_resolver
		$beforeExecution = Get-Date
		
		$psi = New-Object System.Diagnostics.ProcessStartInfo
		$psi.FileName = $aq_query_resolver
		$psi.RedirectStandardOutput = $true
		$psi.RedirectStandardError = $true
		$psi.WorkingDirectory = (Get-Location).Path;
		$psi.UseShellExecute = $false
		$psi.Arguments = "$iniFile test $aq_query_resolver_params"
		$p = New-Object System.Diagnostics.Process
		$p.StartInfo = $psi
		$p.Start() | Out-Null
		
		$output = $p.StandardOutput.ReadToEnd()
		$output += $p.StandardError.ReadToEnd()
		# Write-Output $output
		
		# Write-Output "Page Memory Size:         " $p.PagedMemorySize64
		# Write-Output "Page System Memory Size:  " $p.PagedSystemMemorySize64
		# Write-Output "Peak Paged Memory Size:   " $p.PeakPagedMemorySize64
		# Write-Output "Peak Virtual Memory Size: " $p.PeakVirtualMemorySize64
		# Write-Output "Peak Working Set:         " $p.PeakWorkingSet
		
		$p.WaitForExit()
		
		$afterExecution = Get-Date
		$execTimeMS = New-TimeSpan $beforeExecution $afterExecution
		$execTimeMS = [INT]$execTimeMS.TotalMilliSeconds
        $execTime =   [INT]$execTimeMS/1000
		
		
		#
		# clean
        DeleteExtraFiles $requestAnswerFolderFull
	    
		#
		# copy files
        foreach( $resultFileStart in $resultFilesStart )
        {
			$resultFiles = @(dir ($requestAnswerFolderFull + $resultFileStart + "*") | where {!$_.PsIsContainer } | select -ExpandProperty Name)
			foreach( $resultFile in $resultFiles )
			{
				$src = (Join-Path $requestAnswerFolderFull $resultFile) 
				$dst = $request.BaseName + "_" + $resultFile
				$dst = (Join-Path $testFolder (Join-Path $resultsFolder $dst))
				# Write-Host $src $dst
				Move-Item -force $src $dst
			}
		}
		
		#
		#
		if ($p.ExitCode -ne 0)
		{
			++$warnings
			# ++$errors
			$line = $requestFullName + " WARNING: [" + $aq_query_resolver + " " + $iniFile + " test ] return exit code [" + $p.ExitCode + "]"
			Write-Warning $line
			Add-Content $logFile $line
			
			$content = Get-Content $requestFullName
			Add-Content bad_queries.sql ($content + "`n")
			continue
		}
		
		#
		# check answer
		$checkAnswer=$TRUE
		if ($checkAnswer)
		{
			CheckResult $logfile $requestFullName ($request.BaseName+ "_Answer.txt") $testFolder $execTime ([ref] $errors) ([ref] $unchecked) ([ref] $success) ([ref] $aqEngineErrors)
		}
	}
}
$date = Get-date
Add-Content $logFile ("End: $date")

Write-Output "Done."
Write-Output "$success succeed"
Write-Output "$errors failed (with $aqEngineErrors AQEngine errors)"
Write-Output "$skipped skipped"
Write-Output "$unchecked unchecked"
Write-Output "$noDBErrors with no database"
Write-Output "$warnings warnings"

$total = $success + $errors + $skipped + $unchecked + $noDBErrors + $warnings
Write-Output "$total requests checked"

if ($ouputToNotepad)
{
	&"C:\Windows\System32\notepad.exe"  $logFile
}