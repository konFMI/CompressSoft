#include <iostream>
#include <fstream>
#include <string.h>

#include <queue>

#define DEV 1
#define DEBUG 0
#define HISTSIZE 256

struct HuffmanNode {
	char character;
	int  value;
	int level;
	const HuffmanNode *leftNode;
	const HuffmanNode *rightNode;

	HuffmanNode(char ch = '\0', int val = -1)
	{
		character = ch;
		value = val;
		leftNode = NULL;
		rightNode = NULL;
	}

	HuffmanNode(const HuffmanNode *fNode, const HuffmanNode *sNode)
	{
		character = '\0';
		value     = fNode->value + sNode->value;
		leftNode  = fNode;
		rightNode = sNode;
	}

	bool operator< (const HuffmanNode &node) const
	{
		return value > node.value;
	}
};

static char option;
static std::string pathToInputFile;
static std::string pathToOutputFile;

static std::ifstream inputStream;
static std::ofstream outputStream;

static uint16_t histogram[HISTSIZE];
static uint8_t	hist_size;
static uint8_t 	error_bits;
static std::string huffmanCodeMap[HISTSIZE];

struct HuffmanNode *root = NULL;

void ProcessInput(
	int argc,
	char *argv[]);

void OpenWorkFiles();

void CloseWorkFiles();

void GenerateHistogram();

void WriteHistogram();

void PrintDebugHistogram()
{
	printf("Listing histogram\n");
	for (uint16_t i = 0; i < HISTSIZE; i++)
	{
		if(histogram[(uint8_t)i])
		{
			printf("\t%c\t=\t%d\n", (uint8_t)i, histogram[(uint8_t)i]);		
		}
	}
	printf("End listing\n");
	
}

void PrintDebugCodeMap()
{
	printf("Listing code map\n");
	for (uint16_t i = 0; i < HISTSIZE; i++)
	{
		if(histogram[i])
		{	
			printf("\t%c\t=\t%s\n", (char)i, huffmanCodeMap[i].c_str());
		}
	}
	printf("End listing\n");
}
void BuildHuffmanTree();

std::string GenerateHuffmanCodeMap(
	const HuffmanNode *root,
	std::string code);

void EncodeStream();

void DecodeStream();

std::string HuffmanCompressionGenerateHuffmanCodeMap(
	const HuffmanNode *root,
	std::string code);

int main(
	int argc,
	char *argv[])
{
	ProcessInput(argc, argv);
	OpenWorkFiles();

	GenerateHistogram();
	BuildHuffmanTree();
	GenerateHuffmanCodeMap(root, "");
	PrintDebugCodeMap();

	// By this point everything is set. We can compess or decompress now.
	if (option == 'c')
	{
		EncodeStream();
	}
	else
	{
		DecodeStream();
	}

	CloseWorkFiles();
	return 0;
}

static void ErrorInvalidInputFormat()
{
	std::cerr << "error: Invalid input format.\nusage:\tcompressSoft -[c/d] -[i/o] fileName -[i/o] fileName\n";
	std::cerr << "c : compress\td : decmpress\ni : input\to : output\n";
	exit(1);
}

static void ErrorInvalidOption(
)
{
	std::cerr << "error: Invalid option. Only -c or -d.";
	exit(1);
}

static void ErrorOpenFile(
	std::string pathToInputFile)
{
	std::cerr << "error: failed to open file '" << pathToInputFile << "'\n";
	exit(1);
}

static void HistogramEmptyCheck()
{
	if (hist_size == 0)
	{
		std::cerr << "error:\tNo histogram available. Please check your input file." << std::endl;
		exit(1);
	}
}

void ProcessInput(
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

	option = argv[1][1];
	pathToInputFile = argv[indexToInputFile];
	pathToOutputFile = argv[indexToOutputFile];
}

void OpenWorkFiles()
{
	inputStream.open( pathToInputFile.c_str(), std::ios::in | std::ios_base::binary | std::ios::ate);
	if (!inputStream)
	{
		ErrorOpenFile(pathToInputFile);
	}

	outputStream.open( pathToOutputFile.c_str(), std::ios::out | std::ios_base::binary);
	if (!outputStream)
	{
		ErrorOpenFile(pathToOutputFile);
	}
}

void CloseWorkFiles()
{
	inputStream.close();
	outputStream.close();
}

void ReadCharactersFromInputFile()
{
	std::streampos size;
	char buffer[HISTSIZE];
	if (inputStream.is_open())
	{
    size = inputStream.tellg();
    char *memblock = new char [size];
    inputStream.seekg (0, std::ios::beg);
    inputStream.read (memblock, size);

	for (size_t i = 0; i < size; i++)
	{
		histogram[memblock[i]]++;
	}
	for (size_t i = 0; i < HISTSIZE; i++)
	{
		if(histogram[i])
		{
			hist_size++;
		}
	}
    delete[] memblock;
  }
#if DEBUG
	PrintDebugHistogram();
#endif
}

//TODO: Fine tune a little bit. Or yes.
void ReadHistogramFromInputFile()
{
#if DEBUG
	printf("Reading histogram from input file\n");
#endif
	if (inputStream.is_open())
	{
	    char buffer[1] = {' '};
		inputStream.seekg(0, std::ios_base::beg);
		inputStream.read(buffer, 1);
		error_bits = buffer[0];
		inputStream.read(buffer, 1);
		hist_size = buffer[0];
	}
	if (inputStream.is_open() && hist_size > 0)
	{
		char buffer[2*hist_size] = { 0 };
		inputStream.read(buffer, 2*hist_size);
		for (size_t i = 0; i < 2 * hist_size; i+=2)
		{
			histogram[buffer[i]] = buffer[i+1]; 
		}
#if DEBUG
	PrintDebugHistogram();
#endif
	}
#if DEBUG
	printf("End reading\n");
#endif
}

void GenerateHistogram()
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

//TODO: Tune a little bit more. Or not.
void WriteHistogram()
{
#if DEBUG
	std::cout << "Write histogram\n";
#endif

	char buffer[2 * hist_size] = { 0 };
	buffer[0] = hist_size;
#if DEBUG
	printf("\thist_size = %d\n", (int)hist_size);
	printf("\tbuffer[0] = %d\n", buffer[0]);
#endif

	//outputStream.seekp(0, std::ios_base::beg);
	outputStream.write(buffer, 1);
	for (uint16_t i = 0, j = 0; i < HISTSIZE && j < 2 * hist_size; i++)
	{
		if(histogram[i])
		{
			buffer[j] = i;
			j++;
			buffer[j] = histogram[i];
			j++;		
		}
	}

	outputStream.write(buffer, 2 * hist_size);
#if DEBUG
	printf("Listing buffer histogram\n");
	for (size_t i = 0; i < 2 * hist_size; i+=2)
	{
		printf("\t%c = %d\n", (char)buffer[i], (int)buffer[i+1]);
	}
	
	std::cout << "End writing\n";
#endif
}

void BuildHuffmanTree()
{
	std::priority_queue<HuffmanNode> nodes;
	HuffmanNode *firstMaxNode, *secondMaxNode;
	for (uint16_t i = 0; i < HISTSIZE; i++)
	{
		if (histogram[i])
		{
			nodes.push(HuffmanNode(i, histogram[i]));
		}
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

std::string GenerateHuffmanCodeMap(
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

static int UpdateBitInBuffer(
	char list[],
	int bits,
	int& index,
	int bitValue)
{
	if (index < 0 || index >= bits * HISTSIZE)
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

static int BufferFull(
	int bits,
	int& index)
{
	if (index >= (bits * HISTSIZE))
	{
		return 1;
	}
	return 0;
}

//TODO: Error  bit not handled.
void EncodeStream() 
{
	std::string line;
	char buffer[HISTSIZE] = { 0 };
	int index = 0;

	inputStream.seekg(0, std::ios_base::beg);
	outputStream.write(buffer, 1);
	WriteHistogram();

	while (inputStream.good())
	{
		std::getline(inputStream, line);

		for (char ch : line)
		{
			for (char bit : huffmanCodeMap[ch])
			{
				std::cout << bit;
				UpdateBitInBuffer(buffer, 8, index, bit - '0');
				if (BufferFull(8, index))
				{
					outputStream.write(buffer, HISTSIZE);
					index = 0;
				}
			}
		}
	}

	if (index)
	{
		int minorIndex = index % 8;
		int mainIndex = index / 8;
		printf("\nminorIndex = %d\nmainIndex = %d\nindex = %d\n", minorIndex, mainIndex, index);
		outputStream.write(buffer, mainIndex);
		if (minorIndex)
		{
			outputStream.write(buffer, 1);
			outputStream.seekp(0, std::ios_base::beg);
			buffer[0] = minorIndex;
			outputStream.write(buffer, 1);
		}
	}
}

void DecodeStream()
{
	std::streampos size, pos;
	if (inputStream.good())
	{
		inputStream.seekg(0, std::ios_base::end);
		size = inputStream.tellg();
		pos = 2 * hist_size + 2;
		inputStream.seekg(pos);

		char buffer[HISTSIZE] = { 0 };

		inputStream.read(buffer, size - pos);
		const struct HuffmanNode *ptr = root;
		int index = 0;
		for(int i = 0; i < size - pos; i++)
		{
			uint8_t mask = 1 << 8 - 1;
			while (mask)
			{
				if((buffer[i] & mask) != 0)
				{
					if (ptr->rightNode == NULL)
					{
						std::cout << ptr->character;
						ptr = root->rightNode;
					}
					else
					{
						ptr = ptr->rightNode;
					}
				}
				else
				{
					if (ptr->leftNode == NULL && ptr->rightNode == NULL)
					{
						std::cout << ptr->character;
						ptr = root->leftNode;
					}
					else
					{
						ptr = ptr->leftNode;
					}
				}
				index++;
				mask >>= 1;
			}
		}	
	}
}
