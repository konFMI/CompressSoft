#include <iostream>
#include <fstream>
#include <cstring>
#include <unordered_map>
#include <queue>
#include <vector>
#include <inttypes.h>

static int id = 0;

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
		level = id++;
		leftNode = NULL;
		rightNode = NULL;
	}

	HuffmanNode(const HuffmanNode *fNode, const HuffmanNode *sNode)
	{
		character = '\0';
		value     = fNode->value + sNode->value;
		level = id++;
		leftNode  = fNode;
		rightNode = sNode;
	}

	bool operator< (const HuffmanNode &node) const
	{
		return value > node.value;
	}
};

int validate(int argc, char *argv[]);
void buildTree(HuffmanNode **root, std::unordered_map<char, int> histogram);
void print_BFS(HuffmanNode *root);
std::string generateCodeMap(std::unordered_map<char, std::string> &codeMap, const HuffmanNode *root, std::string code);
void freeTree(HuffmanNode *root);

void printBin(uint8_t num);
void printBinArray_8(std::vector<uint8_t> list);

int UpdateBit(std::vector<uint8_t> &list, int index, uint8_t bitValue);
void write(std::vector<uint8_t> buffer, std::ofstream outStream, int index);

int main(int argc, char *argv[])
{
	if (validate(argc, argv))
	{
		std::cerr << "error:\nusage:\tcompressSoft -[c/d] -[i/o] fileName -[i/o] fileName\n";
		std::cerr << "c : compress\td : decmpress\ni : input\to : output\n";
		return 1;
	}

	char compressOption = argv[1][1];
	int inputFileIndex  = (argv[2][1] == 'i')? 3 : 5;
	int outputFileIndex = (argv[2][1] == 'o')? 3 : 5;

	std::ifstream inStream(argv[inputFileIndex], std::ifstream::in);

	if (!inStream.is_open())
	{
		std::cerr << "failed to open file '" << argv[inputFileIndex] << "'\n";
		return 1;
	}

	std::ofstream outStream(argv[outputFileIndex], std::ofstream::out | std::ofstream::trunc);
	if (!outStream.is_open())
	{
		std::cerr << "failed to open file '" << argv[outputFileIndex] << "'\n";
		return 1;
	}

	std::unordered_map<char, int> histogram;
	std::string line;

	while (inStream.good())
	{
		std::getline(inStream, line);
		for (char ch : line)
		{
			histogram[ch]++;
		}
	}

	for (auto data : histogram)
	{
		std::cout << data.first << "\t" << data.second << std::endl;
	}

	HuffmanNode *root = NULL;

	buildTree(&root, histogram);

	//print_BFS(root);
	std::unordered_map<char, std::string> codeMap;

	(void)generateCodeMap(codeMap, root, "");
	for (auto pair : codeMap)
	{
//		std::cout << pair.first << "\t" << pair.second << std::endl;
	}

	std::vector<uint8_t> buffer(8, 0);
	std::queue<uint8_t> debug;
	int index = 0;
	inStream.clear();
	inStream.seekg(0, std::ios_base::beg);
	std::cout << inStream.good() << std::endl;
	while (inStream.good())
	{
		std::getline(inStream, line);

		for (char ch : line)
		{
			for (char bit : codeMap[ch])
			{
				if (bit == '1')
				{
					while (UpdateBit(buffer, index, 1))
					{

						if (index);
						int majorIndex = index / 8;


						for (int i = 0; i < majorIndex; i++)
						{
							outStream << /*(uint8_t)*/buffer[i];
							debug.push(buffer[i]);
							buffer[i] = 0;
						}
						//write(buffer, outStream, index);
						/*std::cout << std::endl;
						printBinArray_8(buffer);
						std::cout << std::endl;*/
						index = 0;

					}
				}

				if (bit == '0')
				{
					while(UpdateBit(buffer, index, 0))
					{
						int majorIndex = index / 8;

						for (int i = 0; i < majorIndex; i++)
						{
							outStream << (uint8_t)buffer[i];
							debug.push(buffer[i]);
							buffer[i] = 0;
						}
					//	write(buffer, outStream, index);
						/*std::cout << std::endl;
						printBinArray_8(buffer);
						std::cout << std::endl;*/
						index = 0;
					}
				}
			index++;
			}

		}
	}

	//write(buffer, outStream, index);
	int majorIndex = index / 8;
	for (int i = 0; i < majorIndex; i++)
	{
		outStream << (uint8_t)buffer[i];
		debug.push(buffer[i]);
	}

	outStream.close();
	inStream.close();

	//Reading compressed file
	inStream.open(argv[outputFileIndex], std::ios::in);
	if (!inStream.is_open())
	{
		std::cerr << "failed to open file '" << argv[inputFileIndex] << "'\n";
		return 1;
	}

	char  words[8];
	const HuffmanNode *traverseNode = root;
	/*while (inStream.good())
	{
		std::getline(inStream, line);
		
		inStream.read(words, 8);
		for (uint8_t word : words)
		{
			//printBin((uint8_t)word);
			std::cout << word;
			std::cout << "\t";
			//printBin(debug.front());
			std::cout << debug.front();
			debug.pop();
			std::cout << std::endl;
		}*/
		/*for (int i = 7; i >= 0; i--)
		{
			if (word & (1 << i))
			{
				if (traverseNode->rightNode == NULL)
				{
					std::cout << traverseNode->character;
					traverseNode = root;
					i++;
				}
				else
				{
					traverseNode = traverseNode->rightNode;
				}
			}
			else
			{
				if (traverseNode->leftNode == NULL)
				{
					std::cout << traverseNode->character;
					traverseNode = root;
					i++;
				}
				else
				{
					traverseNode = traverseNode->leftNode;
				}
			}
		}*/
	//}

	return 0;
}

int validate(int argc, char *argv[])
{
	if (argc != 6) return 1;

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


	return 0;
}

//TODO: Memory check, if it is release. Not sure if it  is taken care of.
void buildTree(HuffmanNode **root, std::unordered_map<char, int> histogram)
{
	std::priority_queue<HuffmanNode> nodes;

	for (auto pair : histogram)
	{
		nodes.push( HuffmanNode(pair.first, pair.second));
	}

	while (nodes.size() > 1)
	{
		HuffmanNode *firstMaxNode = new HuffmanNode(nodes.top());
		nodes.pop();

		HuffmanNode *secondMaxNode = new HuffmanNode(nodes.top());
		nodes.pop();

		nodes.push(HuffmanNode(firstMaxNode, secondMaxNode));
	}

	*root = new HuffmanNode(nodes.top());

	nodes.pop();
}

std::string generateCodeMap(std::unordered_map<char, std::string> &codeMap, const HuffmanNode *root, std::string code)
{
	if (root == NULL)
	{
		return "";
	}

	if (root->leftNode)
	{
		generateCodeMap(codeMap, root->leftNode, code + "0");
	}
	if (root->rightNode)
	{
		generateCodeMap(codeMap, root->rightNode, code + "1");
	}

	if (root->leftNode == NULL &&
	    root->rightNode == NULL)
	{
		codeMap[root->character] = code;
		//std::cout << "'" << root->character << "'\t" << code<<std::endl;
	}
	return code;
}
void print_BFS(HuffmanNode *root)
{
	std::queue<const HuffmanNode *> nodes;
	const HuffmanNode *currentNode;

	if (root == NULL)
	{
		return;
	}

	nodes.push(root);
	currentNode = root;
	//std::cout << "\tNODE ID\t" << "\tCHARACTER\t" << "VALUE\t"  << std::endl;
	//std::cout  << "\nId:\t" << currentNode->level << "\nRoot address:\t" << currentNode<< "\nRoot character:\t" << currentNode->character << "\nLeft Child address:\t" << currentNode->leftNode << "\nRight Child address:\t" << currentNode->rightNode;

	std::cout << "---------------------------------------------------" << std::endl;
	while (!nodes.empty())
	{
		currentNode = nodes.front();
		nodes.pop();
		//std::cout << "\t" << currentNode->level << "\t\t" << "'" << currentNode->character << "'" << "\t\t"  << currentNode->value << std::endl;
		//std::cout  << "Id:\t" << currentNode->level << "\nNode address:\t" << currentNode<< "\nNode character:\t" << currentNode->character << "\nLeft Child address:\t" << currentNode->leftNode << "\nRight Child address:\t" << currentNode->rightNode;
		//std::cout << std::endl << std::endl;
		if (currentNode->leftNode != NULL)
		{
			nodes.push(currentNode->leftNode);
		}

		if (currentNode->rightNode != NULL)
		{
			nodes.push(currentNode->rightNode);
		}
	}
	std::cout << "---------------------------------------------------" << std::endl;

}

void freeTree(HuffmanNode *root)
{
	//TODO:
}

void printBin(uint8_t num)
{
	/*if (!num) return;
	printBin(num >> 1);*/
	for (int i = 7; i >= 0; i--)
	{
		std::cout << !!(num & (1 << i));
	}
}

void printBinArray_8(std::vector<uint8_t> list)
{

	/*for (auto elm : list)
	{
		int num = elm;
		std::cout << num << "\t\t";
	}*/
	std::cout << std::endl;
	for (auto elm : list)
	{
		std::cout << (int)elm << "\t:\t";
		printBin(elm);
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int UpdateBit(std::vector<uint8_t> &list, int index, uint8_t bitValue)
{
	if (index < 0 || index >= (list.size() * 8))
	{
		return 1;
	}

	int mainIndex = index / 8;

	int minorIndex = index % 8;

	if (!!bitValue)
	{
		list[mainIndex] = list[mainIndex] | (1 << (8-minorIndex - 1));
	}
	else
	{
		list[mainIndex] = list[mainIndex] & ~(1 << (8-minorIndex - 1));
	}

	return 0;
}

void write(std::vector<uint8_t> buffer, std::ofstream outStream, int index)
{
}
