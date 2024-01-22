#include <iostream>
#include <map>
#include <list>
#include <fstream>
#include <vector>

using namespace std;

class HuffmanNode 
{
public:
    char symbol;
    int weight;
    HuffmanNode* left, * right;
    HuffmanNode() { left = NULL; right = NULL;}
    HuffmanNode(HuffmanNode* lef, HuffmanNode* rig)
    {
        left = lef;
        right = rig;
        weight = lef->weight + rig->weight;
    }
    ~HuffmanNode()
    {
        if (left != nullptr) 
        {
            delete left;
            left = nullptr;
        }
        if (right != nullptr) 
        {
            delete right;
            right = nullptr;
        }
    }
};

class Huffman 
{
private:
    HuffmanNode* root_node;
    map<char, int> symbol_frequency;
    map<char, int> ::iterator symbol_frequency_iterator;

    vector<bool> code;
    map<char, vector<bool> > symbol_codewords;

public:
    Huffman();
    ~Huffman();
    void buildTreeEn(ifstream& f);
    void buildTreeDec(ifstream& fg);
    void BuildCode(HuffmanNode* r);
    void encode(ifstream& f, ofstream& g);
    bool decode(ifstream& fg,ofstream& gf);
    struct Sort 
    {
        bool operator() (const HuffmanNode* l, const HuffmanNode* r) 
        {
            return l->weight < r->weight;
        }
    };
};

Huffman::Huffman() 
{
    root_node = NULL;
}

Huffman::~Huffman() 
{
    delete root_node;
    root_node = NULL;
}

void Huffman::buildTreeEn(ifstream& f) 
{
    if (root_node != NULL) 
    {
        delete root_node;
        root_node = NULL;
    }
    while (!f.eof()) 
    {
        char sym = f.get();
        symbol_frequency[sym]++;
    }

    list<HuffmanNode*> nodes;

    for (symbol_frequency_iterator = symbol_frequency.begin(); symbol_frequency_iterator != symbol_frequency.end(); symbol_frequency_iterator++) 
    {
        HuffmanNode* newNode = new HuffmanNode;
        newNode->symbol = symbol_frequency_iterator->first;
        newNode->weight = symbol_frequency_iterator->second;
        nodes.push_back(newNode);
    }

    while (nodes.size() != 1) 
    {
        nodes.sort(Sort());
        HuffmanNode* Left = nodes.front();
        nodes.pop_front();
        HuffmanNode* Right = nodes.front();
        nodes.pop_front();
        HuffmanNode* pr = new HuffmanNode(Left, Right);
        nodes.push_back(pr);
    }
    
    root_node = nodes.front();
}

void Huffman::buildTreeDec(ifstream& f) 
{
    if (root_node != NULL)
    {
        delete root_node;
        root_node = NULL;
    }
    if (symbol_frequency.size() != 0)
        symbol_frequency.clear();
    if (code.size() != 0)
        code.clear();
    if (symbol_codewords.size() != 0)
        symbol_codewords.clear();

    int Count,Weight;
    char sym;
    f.read((char*)&Count, sizeof (Count));

    while (Count > 0) 
    {
        f.read((char*)&sym, sizeof(sym));
        f.read((char*)&Weight, sizeof(Weight));
        Count -= 40;
        symbol_frequency[sym] = Weight;
    }

    list<HuffmanNode*> nodes;

    for (symbol_frequency_iterator = symbol_frequency.begin(); symbol_frequency_iterator != symbol_frequency.end(); symbol_frequency_iterator++)
    {
        HuffmanNode* newNode = new HuffmanNode;
        newNode->symbol = symbol_frequency_iterator->first;
        newNode->weight = symbol_frequency_iterator->second;
        nodes.push_back(newNode);
    }

    while (nodes.size() != 1) 
    {
        nodes.sort(Sort());
        HuffmanNode* Left = nodes.front();
        nodes.pop_front();
        HuffmanNode* Right = nodes.front();
        nodes.pop_front();
        HuffmanNode* pr = new HuffmanNode(Left, Right);
        nodes.push_back(pr);
    }

    root_node = nodes.front();
}

void Huffman::encode(ifstream& f, ofstream& g) 
{
 
    buildTreeEn(f);
    BuildCode(root_node);
    int count = 0;
    for (symbol_frequency_iterator = symbol_frequency.begin(); symbol_frequency_iterator != symbol_frequency.end(); symbol_frequency_iterator++)
        if (symbol_frequency_iterator->second != 0) count += 40;

    g.write((char*)(&count), sizeof(count));

    for (int i = 0; i < 256; i++) {
        if (symbol_frequency[char(i)] > 0) {
            char c = char(i);
            g.write((char*)(&c), sizeof(c));
            g.write((char*)(&symbol_frequency[char(i)]), sizeof(symbol_frequency[char(i)]));
        }
    }

    f.clear();
    f.seekg(0);
    count = 0;

    char temp = 0;
    list<bool> code_list; 
    while (!f.eof()) {
        char c = f.get();
        code_list = list<bool>(symbol_codewords[c].begin(), symbol_codewords[c].end());

        for (bool bit : code_list) {
            temp = temp | bit << (7 - count);
            count++;
            if (count == 8) {
                count = 0;
                g << temp;
                temp = 0;
            }
        }
    }

    f.clear();
    f.seekg(0, std::ios::end);
    g.seekp(0, std::ios::end);
    double sizeF = f.tellg();
    double sizeG = g.tellp();
    f.close();
    g.close();

    double coef = sizeG / sizeF;
    cout << "Compression ratio: " << coef << endl;
}

bool Huffman::decode(ifstream& fg, ofstream& gf)
{
    buildTreeDec(fg);

    if (root_node == nullptr) 
    {
        std::cout << "Error building decoding tree!" << std::endl;
        return false;
    }

    BuildCode(root_node);

    char byte;
    HuffmanNode* currentNode = root_node;

    while (fg.get(byte)) 
    {
        for (int i = 7; i >= 0; --i) 
        {
            bool bit = (byte >> i) & 1;
            currentNode = (bit) ? currentNode->right : currentNode->left;

            if (currentNode == nullptr) 
            {
                std::cout << "Error decoding - Null node encountered!" << std::endl;
                return false;
            }

            if (currentNode->right == nullptr && currentNode->left == nullptr) 
            {
                gf << currentNode->symbol;
                currentNode = root_node;
            }
        }
    }

    return true;
}

void Huffman::BuildCode(HuffmanNode* Node) 
{
    if (Node->left != NULL) 
    {
        code.push_back(0);
        BuildCode(Node->left);
    }

    if (Node->right != NULL) 
    {
        code.push_back(1);
        BuildCode(Node->right);
    }

    if (Node->right == NULL && Node->left == NULL) 
    {
        symbol_codewords[Node->symbol] = code;
    }
    if (!code.empty())
        code.pop_back();
}

int main() 
{
    ifstream f("input.txt", ios::in | ios::binary);
    ofstream g("temp.txt", ios::out | ios::binary);

    Huffman haf;
    haf.encode(f, g);

    f.close();
    g.close();

    ifstream fg("temp.txt", ios::in | ios::binary);
    ofstream gf("output.txt", ios::out | ios::binary);

    Huffman decodeHaf;
    if (decodeHaf.decode(fg, gf))
        cout << "Decoding successful!" << endl;
    else
        cout << "Decoding failed!" << endl;

    fg.close();
    gf.close();

    return 0;
}
