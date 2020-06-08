#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>
#include <bitset>

using namespace std;

class Node
{
public:
    char c;
    int freq;
    bool isLeaf;
    Node *left;
    Node *right;
    Node(char ch, int f, bool leaf = false)
    {
        this->c = ch;
        this->freq = f;
        this->isLeaf = leaf;
        this->left = NULL;
        this->right = NULL;
    }
};

class Comp
{
public:
    bool operator()(Node *a, Node *b)
    {
        return a->freq > b->freq;
    }
};

string getData(string path)
{
    ifstream file(path, ios::in | ios::binary | ios::ate);
    if (file.is_open())
    {
        int size = file.tellg();
        char *memblock = new char[size];
        file.seekg(0, ios::beg);
        file.read(memblock, size);
        file.close();

        string data = memblock;

        delete[] memblock;

        return data;
    }
    else
        cout << "Unable to open file";
}

unordered_map<char, int> calcFreq(string input)
{
    unordered_map<char, int> freq;
    for (auto i : input)
        freq[i]++;

    return freq;
}

Node *genTree(unordered_map<char, int> freq)
{
    priority_queue<Node *, vector<Node *>, Comp> minHeap;

    for (auto p : freq)
    {
        minHeap.push(new Node(p.first, p.second, true));
    }

    while (minHeap.size() > 1)
    {
        Node *root, *l, *r;

        l = minHeap.top();
        minHeap.pop();

        r = minHeap.top();
        minHeap.pop();

        root = new Node('`', l->freq + r->freq);
        root->left = l;
        root->right = r;
        minHeap.push(root);
    }
    return minHeap.top();
}

void storeCodes(Node *root, string code, unordered_map<char, string> &codes)
{
    if (root->isLeaf)
    {
        codes[root->c] = code;
        // cout << root->c << ":" << code << endl;
    }

    else
    {
        storeCodes(root->left, code + "0", codes);
        storeCodes(root->right, code + "1", codes);
    }
}

void addPadding(string &encoded)
{
    int padding_len = 8 - encoded.length() % 8;

    bitset<8> padding_byte(padding_len);

    string padding_code = padding_byte.to_string();

    encoded = padding_code + encoded;

    for (int i = 0; i < padding_len; i++)
        encoded += "0";
}

string encode(string input, unordered_map<char, string> codes)
{
    string encoded = "";

    for (auto ch : input)
        encoded += codes[ch];

    addPadding(encoded);

    return encoded;
}

void writeMeta(unordered_map<char, string> codes)
{
    ofstream file("compressed.bin", ios::out | ios::binary | ios::trunc);
    if (file.is_open())
    {
        int no_of_codes = codes.size();
        file.write(reinterpret_cast<char *>(&no_of_codes), sizeof(no_of_codes));
        for (auto code : codes)
        {
            char letter = code.first;
            char *ch = new char[1];
            ch = &letter;

            file.write(ch, sizeof(ch));
            int code_length = code.second.length();
            file.write(reinterpret_cast<char *>(&code_length), sizeof(code_length));

            file.write(code.second.c_str(), code_length);

            // cout << ch << ":" << code_length << ":" << code.second.c_str();
        }

        file.close();
    }
    else
        cout << "Unable to create file";
}

void writeText(string encoded)
{
    string output_code = "";
    for (int i = 0; i < encoded.length() - 7; i += 8)
    {
        bitset<8> byte(encoded.substr(i, 8));
        output_code += char(byte.to_ulong());
    }

    ofstream file("compressed.bin", ios::out | ios::binary | ios::app);
    if (file.is_open())
    {
        long long OCsize = output_code.size();
        file.write(reinterpret_cast<char *>(&OCsize), sizeof(OCsize));

        // file.write(output_code.c_str(), output_code.size());
        file.close();
    }

    else
        cout << "Unable to open file";
}

void compress()
{
    string path;
    // cin<<path;
    path = "text.txt";
    string input = getData(path);
    unordered_map<char, int> freq = calcFreq(input);
    Node *root = genTree(freq);

    unordered_map<char, string> codes;
    storeCodes(root, "", codes);

    // for (auto p : codes)
    // {
    //     cout << p.first << ":" << p.second << endl;
    // }

    string encoded = encode(input, codes);
    writeMeta(codes);
    writeText(encoded);
    // cout << encoded;
}

int readMeta(string path, unordered_map<string, char> &reverse_map_codes)
{
    ifstream file(path, ios::in | ios::binary);
    if (file.is_open())
    {
        int no_of_codes;
        file.read(reinterpret_cast<char *>(&no_of_codes), sizeof(no_of_codes));
        // cout << "read meta:" << no_of_codes << endl;

        for (int i = 0; i < no_of_codes; i++)
        {
            char *ch = new char[1];
            file.read(ch, sizeof(ch));

            int code_length;
            file.read(reinterpret_cast<char *>(&code_length), sizeof(code_length));

            char *code = new char[code_length + 1];
            file.read(code, code_length);
            code[code_length] = '\0';

            reverse_map_codes[code] = *ch;

            delete[] ch;
            delete[] code;
        }

        int pos = file.tellg();
        file.close();
        return pos;
    }
    else
        cout << "Unable to create file";
    return -1;
}

string readText(int pos, string path)
{
    ifstream file(path, ios::in | ios::binary | ios::ate);
    if (file.is_open())
    {

        file.seekg(pos, ios::beg);
        long long OCsize;
        file.read(reinterpret_cast<char *>(&OCsize), sizeof(OCsize));

        cout << "OCsize:" << OCsize << endl;

        char *memblock = new char[OCsize + 1];
        string coded = "";

        file.read(memblock, sizeof(memblock));

        memblock[OCsize] = '\0';
        coded = memblock;
        file.close();

        cout << "coded:" << memblock << endl;
        return coded;
    }
    else
        cout << "Unable to open file";
    return "";
}

string decode(string encoded, unordered_map<string, char> reverse_map_codes)
{
    int pad = int(encoded[0]);
    string decoded = "";
    for (int i = 1; i < encoded.size(); i++)
    {
        int num_code = int(encoded[i]);
        bitset<8> byte(num_code);
        decoded += byte.to_string();
    }

    decoded.erase(decoded.end() - pad, decoded.end());

    string curr_code = "";
    string text = "";
    for (auto i : decoded)
    {
        curr_code += i;
        if (reverse_map_codes.find(curr_code) != reverse_map_codes.end())
        {
            text += reverse_map_codes[curr_code];
            curr_code = "";
        }
    }

    // cout << "\nDecoded output:" << text;
    return text;
}

void decompress()
{
    string path;
    // cin<<path;
    path = "compressed.bin";
    unordered_map<string, char> reverse_map_codes;

    int pos = readMeta(path, reverse_map_codes);
    cout << pos << endl;

    for (auto p : reverse_map_codes)
    {
        cout << p.first << ":" << p.second << endl;
    }

    string encoded = readText(pos, path);

    // cout << encoded;

    string decoded_text = decode(encoded, reverse_map_codes);
    ofstream file("uncompressed.txt", ios::out | ios::trunc);
    if (file.is_open())
    {
        file << decoded_text;
        file.close();
    }
    else
    {
        cout << "cant create uncompressde file" << endl;
    }
}

int main()
{
    char ch;
    cout << "Compress or decompress(C/D)";
    cin >> ch;
    ch == 'C' ? compress() : decompress();
    return 0;
}
