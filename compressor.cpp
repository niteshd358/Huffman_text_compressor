#include <bits/stdc++.h>
using namespace std;

struct Node {
    char ch;
    int freq;
    Node *left, *right;
    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

// Build Huffman Tree
Node* buildHuffmanTree(unordered_map<char, int> &freq) {
    priority_queue<Node*, vector<Node*>, Compare> pq;
    for (auto &p : freq)
        pq.push(new Node(p.first, p.second));

    while (pq.size() > 1) {
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();
        Node* merged = new Node('\0', left->freq + right->freq);
        merged->left = left;
        merged->right = right;
        pq.push(merged);
    }
    return pq.top();
}

// Generate codes
void generateCodes(Node* root, string code, unordered_map<char,string> &codes) {
    if (!root) return;
    if (!root->left && !root->right) {
        codes[root->ch] = code.empty() ? "0" : code;
    }
    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

// Compress function
void compress(const string &inputFile, const string &outputFile) {
    ifstream in(inputFile);
    if (!in.is_open()) {
        cerr << "Error: cannot open " << inputFile << endl;
        return;
    }

    stringstream buffer;
    buffer << in.rdbuf();
    string text = buffer.str();
    in.close();

    if (text.empty()) {
        cerr << "Error: input file is empty!" << endl;
        return;
    }

    unordered_map<char,int> freq;
    for (char ch : text) freq[ch]++;
    Node* root = buildHuffmanTree(freq);

    unordered_map<char,string> codes;
    generateCodes(root, "", codes);

    string encoded;
    for (char ch : text) encoded += codes[ch];

    ofstream out(outputFile, ios::binary);
    out << codes.size() << "\n";
    for (auto &p : codes) {
        out << (int)(unsigned char)p.first << " " << p.second << "\n";
    }
    out << "---ENDCODES---\n";
    out << encoded;
    out.close();

    cout << "[OK] Compressed '" << inputFile << "' -> '" << outputFile << "'\n";
    cout << "Original size:   " << text.size() * 8 << " bits\n";
    cout << "Compressed size: " << encoded.size() << " bits\n";
    double ratio = 100.0 * (1.0 - (double)encoded.size() / (text.size() * 8));
    cout << "Space saved:     " << fixed << setprecision(2) << ratio << "%\n";
}

// Decompress function
void decompress(const string &inputFile, const string &outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in.is_open()) {
        cerr << "Error: cannot open " << inputFile << endl;
        return;
    }

    int tableSize;
    in >> tableSize;
    string line;
    getline(in, line); // consume newline

    unordered_map<string,char> rev;

    // Read mapping
    for (int i = 0; i < tableSize; i++) {
        getline(in, line);
        if (line == "---ENDCODES---") break;
        stringstream ss(line);
        int ascii;
        string code;
        ss >> ascii >> code;
        rev[code] = (char)ascii;
    }

    // Read until marker
    while (getline(in, line)) {
        if (line == "---ENDCODES---") break;
    }

    // Read encoded bits (rest of file)
    string encoded, chunk;
    while (getline(in, chunk)) {
        encoded += chunk;
    }
    in.close();

    if (encoded.empty()) {
        cerr << "Error: encoded bitstring is empty!" << endl;
        return;
    }

    // Decode
    string decoded, buffer;
    for (char bit : encoded) {
        buffer += bit;
        if (rev.count(buffer)) {
            decoded += rev[buffer];
            buffer.clear();
        }
    }

    ofstream out(outputFile);
    out << decoded;
    out.close();

    cout << "[OK] Decompressed '" << inputFile << "' -> '" << outputFile
         << "' (" << decoded.size() << " chars)\n";
}

// Main CLI
int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage:\n"
             << "  compressor.exe compress <input.txt> <output.huff>\n"
             << "  compressor.exe decompress <input.huff> <output.txt>\n";
        return 1;
    }

    string mode = argv[1];
    if (mode == "compress") {
        compress(argv[2], argv[3]);
    } else if (mode == "decompress") {
        decompress(argv[2], argv[3]);
    } else {
        cerr << "Unknown mode: " << mode << endl;
        return 1;
    }

    return 0;
}
