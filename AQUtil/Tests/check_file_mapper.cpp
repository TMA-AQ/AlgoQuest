#include <aq/FileMapper.h>
#include <aq/Timer.h>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>

std::vector<int> offset_v;

void ReadOffset(const char * pszFileName) 
{
	FILE* prmFile = NULL;
	prmFile = fopen( pszFileName, "rb" );
	offset_v.resize(nb);
	for (unsigned int i = 0; i < nb; i++)
	{
		int prmFileItemSize = 4;
		int theOffset;
		fseek( prmFile, i * prmFileItemSize, SEEK_SET );
		fread( &theOffset, prmFileItemSize, 1, prmFile );
		offset_v[i] = theOffset;
	}
	fclose(prmFile);
}

int main(int argc, char ** argv)
{
	aq::Timer timer;
	ReadOffset(argv[1]);
	std::cout << "Read offset in " << aq::Timer::getString(timer.getTimeElapsed()) << " ms" << std::endl;

	timer.start();
	aq::FileMapper fileMapper(argv[1]);
	int theOffset;
	for (size_t i = 0; i < aq::packet_size; i++)
	{
		fileMapper.read(&theOffset, i * sizeof(int), sizeof(int));
		if (theOffset != offset_v[i])
		{
			std::cerr << "FileMapper return bad value: Get " << theOffset << " Expected " << offset_v[i] << std::endl;
			return EXIT_FAILURE;
		}
	}
	std::cout << "FileMapper read offset in " << aq::Timer::getString(timer.getTimeElapsed()) << " ms" << std::endl;

	return EXIT_SUCCESS;
}