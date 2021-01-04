#ifndef HUFFMANCOMPRESSION_H
#define HUFFMANCOMPRESSION_H
#include <fstream>
#include <string>
#include <inttypes.h>
#include <map>

#define UINT32 uint32_t

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

        int Run(
            int argc,
            char *argv[]);

    private:
        int ProcessInput(
	        int argc,
	        char *argv[]);

        int GenerateHistogram();

        void PrintHistogram() const;

        void WriteHistogram();

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
        void WriteBuffer(uint32_t[], int& );


        std::string                   pathToInputFile;
        std::string                   pathToOutputFile;
        std::ifstream                 inStream;
        std::ofstream                 outStream;
        char                          option; /*Is it encoding or decoding.*/
        std::map<char, UINT32> histogram;
        std::map<char, std::string> huffmanCodeMap;

        struct HuffmanNode *root;
};

#endif //HUFFMANCOMPRESSION_H
