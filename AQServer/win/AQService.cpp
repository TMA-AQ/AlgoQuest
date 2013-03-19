/****************************** Module Header ******************************\ 
* Module Name:  SampleService.cpp 
* Project:      CppWindowsService 
* Copyright (c) Microsoft Corporation. 
*  
* Provides a sample service class that derives from the service base class -  
* CServiceBase. The sample service logs the service start and stop  
* information to the Application event log, and shows how to run the main  
* function of the service in a thread pool worker thread. 
*  
* This source is subject to the Microsoft Public License. 
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL. 
* All other rights reserved. 
*  
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,  
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED  
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE. 
\***************************************************************************/ 

#pragma region Includes 
#include <SQLParser/AQEngine.h>
#include <SQLParser/SQLParser.h>
#include <SQLParser/SQLPrefix.h>
#include <SQLParser/Column2Table.h>
#include <SQLParser/NestedQueries.h>
#include <SQLParser/JeqParser.h>
#include <SQLParser/Exceptions.h>

#include <aq/Server.h>
#include <aq/Configuration.h>
#include <cstdlib>
#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <aq/Logger.h>

#include "AQService.h" 
#include "ThreadPool.h" 
#pragma endregion 

boost::asio::io_service m_io_service;

AQService::AQService(PWSTR pszServiceName,  
										 BOOL fCanStop,  
										 BOOL fCanShutdown,  
										 BOOL fCanPauseContinue) 
										 : CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue) 
{ 
	m_fStopping = FALSE; 

	// Create a manual-reset event that is not signaled at first to indicate  
	// the stopped signal of the service. 
	m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL); 
	if (m_hStoppedEvent == NULL) 
	{ 
		throw GetLastError(); 
	} 
} 


AQService::~AQService(void) 
{ 
	if (m_hStoppedEvent) 
	{ 
		CloseHandle(m_hStoppedEvent); 
		m_hStoppedEvent = NULL; 
	} 
} 


// 
//   FUNCTION: CSampleService::OnStart(DWORD, LPWSTR *) 
// 
//   PURPOSE: The function is executed when a Start command is sent to the  
//   service by the SCM or when the operating system starts (for a service  
//   that starts automatically). It specifies actions to take when the  
//   service starts. In this code sample, OnStart logs a service-start  
//   message to the Application log, and queues the main service function for  
//   execution in a thread pool worker thread. 
// 
//   PARAMETERS: 
//   * dwArgc   - number of command line arguments 
//   * lpszArgv - array of command line arguments 
// 
//   NOTE: A service application is designed to be long running. Therefore,  
//   it usually polls or monitors something in the system. The monitoring is  
//   set up in the OnStart method. However, OnStart does not actually do the  
//   monitoring. The OnStart method must return to the operating system after  
//   the service's operation has begun. It must not loop forever or block. To  
//   set up a simple monitoring mechanism, one general solution is to create  
//   a timer in OnStart. The timer would then raise events in your code  
//   periodically, at which time your service could do its monitoring. The  
//   other solution is to spawn a new thread to perform the main service  
//   functions, which is demonstrated in this code sample. 
// 
void AQService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv) 
{ 
	// Log a service start message to the Application log. 
	WriteEventLogEntry(L"CppWindowsService in OnStart",  
		EVENTLOG_INFORMATION_TYPE); 

	// Queue the main service function for execution in a worker thread. 
	CThreadPool::QueueUserWorkItem(&AQService::ServiceWorkerThread, this); 
} 


// 
//   FUNCTION: CSampleService::ServiceWorkerThread(void) 
// 
//   PURPOSE: The method performs the main function of the service. It runs  
//   on a thread pool worker thread. 
// 
void AQService::ServiceWorkerThread(void) 
{ 

	// tma: FIXME
	std::string cfgFile = "E:/Project/AQServer/config.xml";
	boost::uint16_t port = 9999;

	try
	{
		//
		//
		boost::shared_ptr<aq::Configuration> cfg(new aq::Configuration(cfgFile.c_str()));
		cfg->dump(std::cerr);

		aq::Logger::getInstance().log(AQ_NOTICE, "starting algoquest-server on %u\n", port);
		aq::Server server(m_io_service, port, cfg);

		m_io_service.run();
	}
	catch (const generic_error& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, ex.what());
	}
	catch (const std::exception& ex)
	{
		aq::Logger::getInstance().log(AQ_ERROR, ex.what());
	}

	// Signal the stopped event. 
	SetEvent(m_hStoppedEvent); 
} 


// 
//   FUNCTION: CSampleService::OnStop(void) 
// 
//   PURPOSE: The function is executed when a Stop command is sent to the  
//   service by SCM. It specifies actions to take when a service stops  
//   running. In this code sample, OnStop logs a service-stop message to the  
//   Application log, and waits for the finish of the main service function. 
// 
//   COMMENTS: 
//   Be sure to periodically call ReportServiceStatus() with  
//   SERVICE_STOP_PENDING if the procedure is going to take long time.  
// 
void AQService::OnStop() 
{ 

	m_io_service.stop();

	// Log a service stop message to the Application log. 
	WriteEventLogEntry(L"CppWindowsService in OnStop",  
		EVENTLOG_INFORMATION_TYPE); 

	// Indicate that the service is stopping and wait for the finish of the  
	// main service function (ServiceWorkerThread). 
	m_fStopping = TRUE; 
	if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0) 
	{ 
		throw GetLastError(); 
	} 
}