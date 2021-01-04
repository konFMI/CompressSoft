#include "HuffmanCompression.h"

#include <iostream>
#include <cstring>
#include <inttypes.h>
#include <queue>
#include <vector>

#define SIZE 32

static void PrintInputStreamStateMessage(
	std::ifstream &stream)
{
	std::cout << "Input stream is " << ((stream.good())? "good" : "bad") << std::endl;
}
static void PrintInputErrorMessage()
{
	std::cerr << "error:\nusage:\tcompressSoft -[c/d] -[i/o] fileName -[i/o] fileName\n";
	std::cerr << "c : compress\td : decmpress\ni : input\to : output\n";
}

static void PrintOpenFileErrorMessage(
	std::string pathToInputFile)
{
	std::cerr << "failed to open file '" << pathToInputFile << "'\n";
}

static void ClearBuffer(uint32_t buffer[], int count)
{
	for (size_t i = 0; i < count; i++)
	{
		buffer[i] = 0;
	}
	
}

static void BinaryPrint(uint32_t num)
{
	uint32_t mask = 1 << 31;

	while (mask)
	{
		std::cout << ((num & mask) != 0);
		mask >>= 1;
	}
	std::cout << std::endl;
}

static void PrintBinaryBuffer(uint32_t list[], int index)
{
	std::cout << "PRINT START" << std::endl;
	std::cout << "index : " << index << std::endl;
	for (int i = 0; i <= index / 32 && i < SIZE; i++)
	{
		std::cout << list[i] << "\t";
		BinaryPrint(list[i]);
	}
	std::cout << "PRINT END" << std::endl;
}

static void PrintCodeTable(std::map<char, std::string> huffmanCodeMap)
{
	for (auto pair : huffmanCodeMap)
	{
		std::cout << pair.first << "\t" << pair.second << std::endl;
	}
}

static int BufferFull(uint32_t list[], int& index)
{
	if (index >= (SIZE * 32))
	{
		return 1;
	}
	return 0;
}

static int UpdateBitInBuffer(uint32_t list[], int& index, uint8_t bitValue)
{
	int bits = 32;
	if (index < 0 || index >= SIZE * bits)
	{
		return 1;
	}


	int minorIndex = index % bits;
	int mainIndex = index / bits;

	if (!!bitValue)
	{
		list[mainIndex] = list[mainIndex] | (1 << (bits - minorIndex - 1));
	}
	else
	{
		list[mainIndex] = list[mainIndex] & ~(1 << (bits - minorIndex - 1));
	}

	index++;
	return 0;
}

HuffmanCompression::HuffmanCompression()
{
	root = nullptr;
}

HuffmanCompression::~HuffmanCompression()
{
	inStream.close();
	outStream.close();

	DeleteTree(root);
}

int HuffmanCompression::Run(int argc, char *argv[])
{
    if (ProcessInput(argc, argv))
	{
		PrintInputErrorMessage();
		return 1;
	}

	this->inStream.open(pathToInputFile, std::ifstream::in);
	if (!inStream.is_open())
	{
		PrintOpenFileErrorMessage(pathToInputFile);
		return 1;
	}

	this->outStream.open(pathToOutputFile, std::ofstream::out | std::ofstream::trunc);
	if (!outStream.is_open())
	{
		PrintOpenFileErrorMessage(pathToOutputFile);
		return 1;
	}

	if (GenerateHistogram())
	{
		return 1;
	}

	BuildHuffmanTree();
	GenerateHuffmanCodeMap(root, "");

	// By this point everything is set. We can compess or decompress now.
	if (option == 'c')
	{
		WriteHistogram();
		EncodeStream();
	}
	else if (option == 'd')
	{

	}

    return 0;
}

/* Check if the input arguments are in the expected format. */
int HuffmanCompression::ProcessInput(
	int argc,
	char *argv[])
{
	/* Check argument length. */
	if (argc != 6) return 1;

	/* Check individual arguments lenght. */
	if (strlen(argv[1]) != 2 ||
	    strlen(argv[2]) != 2 ||
	    strlen(argv[4]) != 2)
	 {
	 	return 1;
	 }

	/* First option for -[c/d] */
	if (argv[1][0] != '-' ||
	   (argv[1][1] != 'c' &&
	    argv[1][1] != 'd'))
	 {
	 	return 1;
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
		return 1;
	 }

	int indexToInputFile  = (argv[2][1] == 'i')? 3 : 5;
	int indexToOutputFile = (argv[2][1] == 'o')? 3 : 5;


	this->option = argv[1][1];
	this->pathToInputFile = argv[indexToInputFile];
	this->pathToOutputFile = argv[indexToOutputFile];

	return 0;
}

int HuffmanCompression::GenerateHistogram()
{
	std::string line;
	//TODO: Depending on the option value, the histogram will be generated
	//		by a text for encoding or a from an encoded file.
	//		In one way it is straightforward. The other needs more attention.

	//Straightforward way
	switch (option)
	{
	case 'c':
		while (inStream.good())
		{
			std::getline(inStream, line);
			for (char ch : line)
			{
				this->histogram[ch]++;
			}
		}
		break;
	case 'd':
		ReadHistogramFromInputFile();
		break;
	default:
		return 1;
	}

	return 0;
}

void HuffmanCompression::PrintHistogram() const
{
	for (auto pair : histogram)
	{
		if (pair.second)
		{
			std::cout << pair.first << "\t" << pair.second << std::endl;
		}
	}
}

void HuffmanCompression::WriteHistogram()
{
	ResetToBeginningOutuputStream();

	outStream << histogram.size() << std::endl;
	for ( auto pair : histogram)
	{
		outStream << pair.first << pair.second << std::endl;
	}
}

void HuffmanCompression::ReadHistogramFromInputFile()
{
	int size = 0;
	char character;
	UINT32 value = 0;

	if (inStream.good())
	{
		inStream >> size;
	}

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
}

void HuffmanCompression::ResetToBeginningInputStream()
{
	this->inStream.clear();
	this->inStream.seekg(0, std::ios_base::beg);
}

void HuffmanCompression::ResetToBeginningOutuputStream()
{
	this->outStream.clear();
	this->outStream.seekp(0,std::ios_base::beg);
}

void HuffmanCompression::BuildHuffmanTree()
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

void HuffmanCompression::EncodeStream() 
{

	std::string line;
	uint32_t buffer[SIZE] = { 0 };
	int index = 0;
	
	ResetToBeginningInputStream();

	PrintCodeTable(huffmanCodeMap);
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
					WriteBuffer(buffer, index);
				}
			}
		}
	}
	WriteBuffer(buffer, index);
}

void HuffmanCompression::WriteBuffer(uint32_t list[], int& index)
{
	int bits = 32;
	uint8_t mainIndex = 0;
	uint8_t errorBits = 0;
	
	mainIndex = index / bits;
	errorBits = index % bits;
	
	for	(int i = 0; i <= mainIndex; i++)
	{
		outStream << list[i];
	}
	if (errorBits)
	{
		std::cout << errorBits << std::endl;
		PrintBinaryBuffer(list, mainIndex + 1 );
	}
	if (errorBits)
	{
		uint32_t tmp = list[mainIndex + 1];
		ClearBuffer(list, SIZE);
		list[0] = tmp;
		index = errorBits;
	}
	else
	{
		ClearBuffer(list, SIZE);
		index = 0;
	}
	if (errorBits)
	PrintBinaryBuffer(list, mainIndex + 1 );

}
