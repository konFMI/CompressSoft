#include "HuffmanCompression.h"

#include <iostream>
#include <cstring>
#include <inttypes.h>
#include <queue>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEBUG 0
#define PRINT_STARS()(std:: cout << "\n*****************************************\n")

static void BinaryPrint(
	UINTBITMAP num)
{
	UINTBITMAP mask = 1 << BITMAPENTRYBITS - 1;

	while (mask)
	{
		std::cout << ((num & mask) != 0);
		mask >>= 1;
	}
	
	std::cout << "\n";
}

static void PrintBinaryBuffer(
	UINTBITMAP list[],
	UINTBITMAP index)
{
	std::cout << "\nindex =\t" << (int)index << "\n";
	for (int i = 0; i < index / BITMAPENTRYBITS && i < SIZE; i++)
	{
		std::cout << list[i] << "\t";
		BinaryPrint(list[i]);
	}

}

static void PrintCodeTable(
	std::map<char, std::string> huffmanCodeMap)
{
	for (auto pair : huffmanCodeMap)
	{
		std::cout << pair.first << "\t" << pair.second << std::endl;
	}

	PRINT_STARS();
}

static void PrintHistogram(
	std::map<char, UINTHISTVALUE> histogram)
{
	for (auto pair : histogram)
	{
		if (pair.second)
		{
			std::cout << pair.first << "\t" << pair.second << std::endl;
		}
	}
}

static void PrintInputStreamStateMessage(
	std::ifstream &stream)
{
	std::cout << "Input stream is " << ((stream.good())? "good" : "bad") << std::endl;
}

static void ErrorOpenFile(
	std::string pathToInputFile)
{
	std::cerr << "error: failed to open file '" << pathToInputFile << "'\n";
	exit(1);
}

static void ErrorInvalidOption(
)
{
	std::cerr << "error: Invalid option. Only -c or -d.";
	exit(1);
}

static void ErrorInvalidInputFormat(
)
{
	std::cerr << "error: Invalid input format.\nusage:\tcompressSoft -[c/d] -[i/o] fileName -[i/o] fileName\n";
	std::cerr << "c : compress\td : decmpress\ni : input\to : output\n";
	exit(1);
}

static void HistogramEmptyCheck(
	std::map<char, UINTHISTVALUE>& histogram)
{
	if (histogram.empty())
	{
		std::cerr << "error:\tNo histogram available. Please check your input file." << std::endl;
		exit(1);
	}
}

static int BufferFull(
	UINTBITMAP list[],
	UINTBITMAP& index)
{
	if (index >= (SIZE * BITMAPENTRYBITS))
	{
		return 1;
	}
	return 0;
}

static int UpdateBitInBuffer(
	UINTBITMAP list[],
	UINTBITMAP& index,
	UINTBITMAP bitValue)
{
	if (index < 0 || index >= SIZE * BITMAPENTRYBITS)
	{
		return 1;
	}


	int minorIndex = index % BITMAPENTRYBITS;
	int mainIndex = index / BITMAPENTRYBITS;

	if (!!bitValue)
	{
		list[mainIndex] = list[mainIndex] | (1 << (BITMAPENTRYBITS - minorIndex - 1));
	}
	else
	{
		list[mainIndex] = list[mainIndex] & ~(1 << (BITMAPENTRYBITS - minorIndex - 1));
	}

	index++;
	return 0;
}

static void ClearBuffer(
	UINTBITMAP buffer[],
	int count)
{
	for (size_t i = 0; i < count; i++)
	{
		buffer[i] = 0;
	}
	
}

HuffmanCompression::HuffmanCompression(
)
{
	root = nullptr;
	inputFd	 = -1;
	outputFd = -1;
}

HuffmanCompression::~HuffmanCompression(
)
{
	inStream.close();
	outStream.close();

	close(inputFd);
	close(outputFd);

	DeleteTree(root);
}

void HuffmanCompression::Run(
	int argc,
	char *argv[])
{
   	ProcessInput(argc, argv);
	OpenWorkFiles();

	GenerateHistogram();
	BuildHuffmanTree();
	GenerateHuffmanCodeMap(root, "");
	
	// By this point everything is set. We can compess or decompress now.
	if (option == 'c')
	{
		EncodeStream();
	}
	else
	{
		DecodeStream();
	}
}

void HuffmanCompression::ProcessInput(
	int argc,
	char *argv[])
{
	/* Check argument length. */
	
	/* Check individual arguments lenght. */
	if (argc 			!= 6 ||
		strlen(argv[1]) != 2 ||
	    strlen(argv[2]) != 2 ||
	    strlen(argv[4]) != 2)
	 {
	 	ErrorInvalidInputFormat();
	 }

	/* First option for -[c/d] */
	if (argv[1][0] != '-' ||
	   (argv[1][1] != 'c' &&
	    argv[1][1] != 'd'))
	 {
		 ErrorInvalidOption();
	 }

	/* Second and third option for -[i/o] */
	if (argv[2][0] != '-'  ||
	   (argv[2][1] != 'i'  &&
	    argv[2][1] != 'o') ||
	   (argv[4][1] != 'i'  &&
	    argv[4][1] != 'o') ||
	   (argv[2][1] == 'i'  &&
	    argv[4][1] == 'i') ||
	   (argv[2][1] == 'o'  &&
	    argv[4][1] == 'o'))
	 {
		ErrorInvalidInputFormat();
	 }

	int indexToInputFile  = (argv[2][1] == 'i')? 3 : 5;
	int indexToOutputFile = (argv[2][1] == 'o')? 3 : 5;

	this->option = argv[1][1];
	this->pathToInputFile = argv[indexToInputFile];
	this->pathToOutputFile = argv[indexToOutputFile];
}

void HuffmanCompression::OpenWorkFiles()
{
	this->inStream.open(pathToInputFile, std::ifstream::in);
	if (!inStream.is_open())
	{
		ErrorOpenFile(pathToInputFile);
	}

	inputFd = open( pathToInputFile.c_str(), O_RDONLY);
	if (inputFd == -1)
	{
		ErrorOpenFile(pathToInputFile);
	}

	outputFd = open( pathToOutputFile.c_str(), O_WRONLY);
	if (outputFd == -1)
	{
		ErrorOpenFile(pathToOutputFile);
	}
	

	this->outStream.open(pathToOutputFile, std::ofstream::out | std::ofstream::trunc);
	if (!outStream.is_open())
	{
		ErrorOpenFile(pathToOutputFile);
	}
}

void HuffmanCompression::GenerateHistogram(
)
{
	//TODO: Depending on the option value, the histogram will be generated
	//		by a text for encoding or a from an encoded file.
	//		In one way it is straightforward. The other needs more attention.
	//DONE!
	switch (option)
	{
	case 'c':
		ReadCharactersFromInputFile();
		break;
	case 'd':
		ReadHistogramFromInputFile();
		break;
	default:
		ErrorInvalidOption();
	}
}

void HuffmanCompression::BuildHuffmanTree(
)
{
	std::priority_queue<HuffmanNode> nodes;
	HuffmanNode *firstMaxNode, *secondMaxNode;
	for (auto pair : histogram)
	{
		nodes.push(HuffmanNode(pair.first, pair.second));
	}

	while (nodes.size() > 1)
	{
		firstMaxNode = new HuffmanNode(nodes.top());
		nodes.pop();

		secondMaxNode = new HuffmanNode(nodes.top());
		nodes.pop();

		nodes.push(HuffmanNode(firstMaxNode, secondMaxNode));
	}
	
	root = new HuffmanNode(nodes.top());
	
	nodes.pop();
}

std::string HuffmanCompression::GenerateHuffmanCodeMap(
	const HuffmanNode *root,
	std::string code)
{
	if (root == NULL)
	{
		return "";
	}

	if (root->leftNode)
	{
		GenerateHuffmanCodeMap(root->leftNode, code + "0");
	}
	if (root->rightNode)
	{
		GenerateHuffmanCodeMap(root->rightNode, code + "1");
	}

	if (root->leftNode == NULL &&
	    root->rightNode == NULL)
	{
		huffmanCodeMap[root->character] = code;
	}
	return code;
}

void HuffmanCompression::EncodeStream(
) 
{
	std::string line;
	UINTBITMAP buffer[SIZE] = { 0 };
	UINTBITMAP index = 0;
	
	WriteHistogram();
	#if DEBUG
	
	ResetToBeginningInputStream();

	while (inStream.good())
	{
		std::getline(inStream, line);

		for (char ch : line)
		{
			for (char bit : huffmanCodeMap[ch])
			{
				UpdateBitInBuffer(buffer, index, bit - '0');
				if (BufferFull(buffer, index))
				{
					#if DEBUG
					std::cout << "Buffer is full.\nPrinting buffer...";
					PrintBinaryBuffer(buffer, index);
					std::cout << "DONE\nWriting buffer to file...\n";
					#endif

					WriteBuffer(buffer, index);
					
					#if DEBUG
					std::cout << "DONE\nPrint buffer after writing to file...\n";
					PrintBinaryBuffer(buffer, index);
					#endif
				}
			}
		}
	}
	WriteBuffer(buffer, index);
	if (index)
	{
		outStream.seekp(0);
		outStream << (UINTBITMAP)index;
	}
#endif
}

void HuffmanCompression::DecodeStream(
)
{
#if DEBUG
	PrintHistogram(this->histogram);
#endif
}

void HuffmanCompression::ResetToBeginningInputStream(
)
{
	this->inStream.clear();
	this->inStream.seekg(0, std::ios_base::beg);
}

void HuffmanCompression::ResetToBeginningOutuputStream(
)
{
	this->outStream.clear();
	this->outStream.seekp(0,std::ios_base::beg);
}

void HuffmanCompression::ReadCharactersFromInputFile(
)
{
	std::string line;
	while (inStream.good())
	{
		std::getline(inStream, line);
		for (char ch : line)
		{
			this->histogram[ch]++;
		}
	}
	
}

void HuffmanCompression::ReadHistogramFromInputFile(
)
{
	int size = 0;
	char character;
	UINTHISTVALUE value = 0;
	UINTBITMAP errorBits = 0;

	if (inStream.good())
	{
		inStream >> size;
	}
#if DEBUG
	std::cout << "ReadHistogramFromInputFile:" << size << std::endl;
	std::cout << "size = " << size << std::endl;
#endif
	while (inStream.good() && size > 0)
	{
		if (inStream.peek() == '\n')
		{
			inStream >> std::noskipws >> character;
		}
		inStream >> std::noskipws >> character;
		if (inStream.good())
		{
			inStream >> std::noskipws >> value;
			histogram[character] = value;
		}
		size--;
	}
#if DEBUG
	PrintHistogram(this->histogram);
#endif
	HistogramEmptyCheck(this->histogram);

}

void HuffmanCompression::WriteHistogram(
)
{
	ResetToBeginningOutuputStream();
#if DEBUG
	std::cout << "WriteHistogram: ";
	std::cout << "histogram.size() = " << histogram.size() << std::endl;
#endif
	uint16_t 		count[1];
	size_t 			i;
	char 			*characters 	= new char[count[0]];
	UINTHISTVALUE 	*value			= new UINTHISTVALUE[count[0]]; 
	print	f("%d\n", count[0]);
/*
	for (auto pair : histogram)
	{
		if (i < count[1])
		{
			characters[i]	= pair.first;
			value[i] 		= pair.second;
			i++;
		}
	}
	
	for ( i = 0; i < count[1]; i++)
	{
		printf("%c\t%d\n", characters[i], value[i]);
	}
*/	
	//write(outputFd, count, 1);
	delete [] value;
	delete [] characters;
}

void HuffmanCompression::WriteBuffer(
	UINTBITMAP list[],
	UINTBITMAP& index)
{
#if DEBUG
	std::cout << "WriteStart\n";
#endif
	UINTBITMAP mainIndex = 0;
	UINTBITMAP errorBits = 0;
	
	mainIndex = index / BITMAPENTRYBITS;
	errorBits = index % BITMAPENTRYBITS;


	for	(int i = 0; i <= mainIndex; i++)
	{
		outStream << (UINTBITMAP)list[i];
	}
	
	if (errorBits)
	{
		UINTBITMAP tmp = list[mainIndex];
		ClearBuffer(list, SIZE);
		list[0] = tmp;
		index = errorBits;
	}
	else
	{
		ClearBuffer(list, SIZE);
		index = 0;
	}
#if DEBUG
	std::cout << "WriteEnd\n";
#endif
}