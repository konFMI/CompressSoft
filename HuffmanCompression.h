#ifndef HUFFMANCOMPRESSION_H
#define HUFFMANCOMPRESSION_H
#include <fstream>
#include <string>
#include <inttypes.h>
#include <map>
#include <unistd.h>

#define SIZE 8
#define BITMAPENTRYBITS 8
#define UINTBITMAP uint8_t
#define UINTHISTVALUE uint8_t

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

class HuffmanCompression {
    public:
        HuffmanCompression();

        ~HuffmanCompression();

        void Run(
            int argc,
            char *argv[]);

    private:
        void ProcessInput(
	        int argc,
	        char *argv[]);

        void OpenWorkFiles();

        void GenerateHistogram();

        void WriteHistogram();

        void ReadCharactersFromInputFile();

        void ReadHistogramFromInputFile();

        void ResetToBeginningInputStream();

        void ResetToBeginningOutuputStream();

        void BuildHuffmanTree();

        void DeleteTree(
            const HuffmanNode* r)
        {
            if (r)
            {
                DeleteTree(r->leftNode);
                DeleteTree(r->rightNode);

                delete r;
            }   
        }

        std::string GenerateHuffmanCodeMap(
	        const HuffmanNode *root,
	        std::string code);

        void EncodeStream();
	    void DecodeStream();
        void WriteBuffer(UINTBITMAP[], UINTBITMAP& );


        std::string                   pathToInputFile;
        std::string                   pathToOutputFile;

        std::ifstream                 inStream;
        std::ofstream                 outStream;

        int                           inputFd;
        int                           outputFd;

        char                          option; /*Is it encoding or decoding.*/
        std::map<char, UINTHISTVALUE> histogram;
        std::map<char, std::string>   huffmanCodeMap;

        struct HuffmanNode *root;
};

#endif //HUFFMANCOMPRESSION_H
