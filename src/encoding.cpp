/**
  * By: Pablo Rodriguez Bertorello
  * Note: EXTRA FEATURE feature added, compacting the header in simpler form
  *
  * References:
  * - tree from string https://www.geeksforgeeks.org/construct-binary-tree-string-bracket-representation/
  * - tree to array https://nptel.ac.in/courses/106103069/51
  **/

#include "encoding.h"
#include "priorityqueue.h"

#include "stack.h"
#include "vector.h"

#include <string>
#include <iostream>

// interface additional to header
PriorityQueue<HuffmanNode*> getQueue(Map<char, int>& freqTable);
void buildEncodingMap(HuffmanNode* node, Map<char, string>& encodingMap, string path);
bool isBeginLeaf(string encoding, int index);
bool isEndLeaf(string encoding, int index);
int getEncodingCount(string encoding);
char getDecodedChar(string encoding, int index);
string getDecodedPath(string encoding, int index);
void recreateTreeFromHeader(string header, HuffmanNode*& root, char key, string path);
void recreateTreeFromHeader(string header, HuffmanNode*& root, Vector<char> keys, Vector<string> paths);
void compressLine(string line, Map<char, string>& encodingMap, HuffmanOutputFile& output);
void compressFile(istream& input, string header, Map<char, string>& encodingMap, HuffmanOutputFile& output);
bool isValidValueL2(string encodingValue, Map<char, string>& encodingMap);
bool isValidValueL1(string encodingValue, Map<char, string>& encodingMap, Map<string, bool>& isValidCache);
char getValidKey(string encodingValue, Map<char, string>& encodingMap);
char getValidKey(string encodingValue, Map<char, string>& encodingMap, Map<string, char>& charCache);
void writeBit(char ch, ostream& output);
void decompressFile(HuffmanInputFile& input, Map<char, string>& encodingMap, ostream& output);

const char DEFAULT_NODE_CHAR = '\0'; // per HuffmanNode.cpp

/**
 * @brief buildFrequencyTable builds frequency table from input stream
 * @param input
 * @return
 */
Map<char, int> buildFrequencyTable(istream& input)
{

    Map<char, int> frequency = Map<char, int>();

    char ch;
        while (input.get(ch)) { //  true if successful, false if fail/EOF
            if(!frequency.containsKey(ch)) {
                frequency.put(ch, 1);
            } else {
                int current = frequency.get(ch);
                frequency.put(ch, current+1);
            }
        }

    return frequency;
}


/**
 * @brief getQueue creates a queue from a frequency table
 * @param freqTable
 * @return
 */
PriorityQueue<HuffmanNode*> getQueue(Map<char, int>& freqTable) {
    PriorityQueue<HuffmanNode*> queue = PriorityQueue<HuffmanNode*>();

    for(char key:freqTable.keys()) {
        HuffmanNode* node = new HuffmanNode(key);
        double priority = 1.0 * freqTable.get(key); // min-heap queue
        queue.enqueue(node, priority);
    }
    return queue;
}


/**
 * @brief buildEncodingTree builds encoding tree from frequency table
 * @param freqTable
 * @return
 */
HuffmanNode* buildEncodingTree(Map<char, int>& freqTable)
{
    // http://stanford.edu/~stepp/cppdoc/PriorityQueue-class.html
    PriorityQueue<HuffmanNode*> queue = getQueue(freqTable);

    while(queue.size()>1) {
        double zeroPriority = queue.peekPriority();
        HuffmanNode* zeroNode = queue.dequeue();
        double onePriority = queue.peekPriority();
        HuffmanNode* oneNode = queue.dequeue();
        double jointPriority = zeroPriority + onePriority;
        HuffmanNode* parent = new HuffmanNode(zeroNode, oneNode);
        queue.enqueue(parent, jointPriority);
    }

    HuffmanNode* peek = queue.peek();  // largest

    return peek;
}


/**
 * @brief buildEncoding recursively finds the code for each character
 * based on their position in the encoding tree
 * @param node
 * @param encodingMap
 * @param path
 */
void buildEncodingMap(HuffmanNode* node, Map<char, string>& encodingMap, string path)
{
    if(node == nullptr) { // base
        return;
    }

    // left
    buildEncodingMap(node->zero, encodingMap, path+"0");

    // this
    if(node->zero == nullptr &&
            node->one == nullptr) {

        char key = node->ch;

        if(key != DEFAULT_NODE_CHAR) {
            encodingMap.put(key, path);
        }
    }

    // right
    buildEncodingMap(node->one, encodingMap, path+"1");
}


/**
 * @brief buildEncodingMap builds encoding map from tree
 * e.g. 'A':  "10"
 * @param encodingTree
 * @return
 */
Map<char, string> buildEncodingMap(HuffmanNode* root)
{
    Map<char, string> encoding = Map<char, string> ();
    buildEncodingMap(root, encoding, "");

    return encoding;
}


/**
 * @brief isBeginLeaf in a header, identifies where a leaf node encoding's starts
 * @param encoding
 * @param index
 * @return
 */
bool isBeginLeaf(string encoding, int index) {
    int outer = index+2;
    if(outer < encoding.size()) { // bounds
        return encoding[index] == '(' && encoding[outer] == '.';
    }
    return false;
}


/**
 * @brief isEndLeaf in a header, identifies where a leaf node encoding's ends
 * @param encoding
 * @param index
 * @return
 */
bool isEndLeaf(string encoding, int index) {
    if(encoding[index] == ')' && index==encoding.size()-1) { // end of header
        return true;
    }
    int outer = index+1;
    if(outer < encoding.size()) { // bounds
        return encoding[index] == ')' && encoding[outer] == '(' ;
    }
    return false;
}


/**
 * @brief getTreeDepth determines the depth of a tree from it's header encoding
 * @param encoding
 * @return
 */
int getEncodingCount(string encoding) {
    int count = 0;
    for(int i=0; i<encoding.size(); i++) {
        if(isBeginLeaf(encoding, i)) { // pattern match

            count++;
        }
    }
    return count;
}


/**
 * @brief getTreeDepth gets the ith code in the encoding array
 * Example: from (g.10111) -> g
 * @param encoding
 * @param index
 * @return
 */
char getDecodedChar(string encoding, int index) {

    int leafCount=0;

    for(int i=0; i<encoding.size(); i++) {

        string snipet = encoding.substr(i, 4); // for debug

        if(isBeginLeaf(encoding, i) && leafCount==index) {  // the one looking for
            char decodedChar = encoding[i+1];
            return decodedChar;
        }

        if(isBeginLeaf(encoding, i)) {
            leafCount++;
        }

    }

    return DEFAULT_NODE_CHAR;
}


/**
 * @brief getDecodedPath finds the leaf node's encoded branch
 * Example: from (g.10111) -> 10111
 * @param encoding
 * @param index
 * @return
 */
string getDecodedPath(string encoding, int index) {

    int leafCount=0;

    for(int i=0; i<encoding.size(); i++) {

        string snipet = encoding.substr(i, 4);  // for debug

        if(isBeginLeaf(encoding, i) && leafCount==index) {

            for(int j=i; j<encoding.size(); j++) {
                if(isEndLeaf(encoding, j)) {
                    int pathBegins = i+3;
                    int pathEnds = j;
                    string decodedPath = encoding.substr(pathBegins, pathEnds-pathBegins);
                    return decodedPath;
                }

            }
        }

        if(isBeginLeaf(encoding, i)) {
            leafCount++;
        }

    }

    return "DEFAULT_NODE_CHAR";
}


/**
 * @brief flattenTreeToHeader recursively, flatten the tree into string
 * that can be used to recreate the tree
 * @param node
 * @return
 */
string flattenTreeToHeader(HuffmanNode* root) {

    string header = "";

    Map<char, string> encodingMap = buildEncodingMap(root);

    for(char key:encodingMap.keys()) { // all other keys will be later assumed to be null (not a leaf node)
        string value = encodingMap.get(key);
        string node = string(1, key);
        string increment = "";
        if(key != '\0') { // leaf
            increment += "(" + node + "." + value + ")";
        }
        header += increment;
        // cout << "encoded[" << node << "]=" << increment << endl;
    }

    // for(int j=0; j<getEncodingCount(header); j++) { // testing
         // cout << "decoded[" << j << "]=" << getDecodedChar(header, j) << " path=" << getDecodedPath(header, j) << endl;
    // }

    return header;
}


/**
 * @brief recreateTreeFromHeader places a leaf node in a tree according to its path
 * It adds non-leaf nodes as necessary to be able to place the leaf
 * @param header
 * @param root
 * @param key
 * @param path
 */
void recreateTreeFromHeader(string header, HuffmanNode*& root, char key, string path) {

    if(path == "") { // base case
        return;
    }

    char currentNode = path[0];

    if(path.size() == 1) { // leaf node
        if(currentNode == '0') {
            root->zero = new HuffmanNode(key);
            return;
        }
        root->one = new HuffmanNode(key);
        return;
    }

    // branch node
    string shortened = path.substr(1, path.size()-1);

    if(currentNode == '0') {
        if(root->zero == nullptr) { // was not added before
            root->zero = new HuffmanNode(nullptr, nullptr);
        }
        recreateTreeFromHeader(header, root->zero, key, shortened);
    } else {
        if(root->one == nullptr) {  // was not added before
            root->one = new HuffmanNode(nullptr, nullptr);
        }
        recreateTreeFromHeader(header, root->one, key, shortened);
    }
}


/**
 * @brief recreateTreeFromHeader iterates over the characters in the header, placing a leaf in the tree for each
 * @param header
 * @param root
 * @param keys
 * @param paths
 */
void recreateTreeFromHeader(string header, HuffmanNode*& root, Vector<char> keys, Vector<string> paths) {

    for(int i=0; i<keys.size(); i++) {
        char key = keys.get(i);
        string path = paths.get(i);
        recreateTreeFromHeader(header, root, key, path);
    }

}


/**
 * @brief recreateTreeFromHeader recursively recreates the tree from its encoding
 * @param encoding examples: "(a.0001)(b.0000)(c.0011)(d.0010)"
 * @return
 */
void recreateTreeFromHeader(string header, HuffmanNode*& root)
{
    Vector<char> keys = Vector<char>();
    Vector<string> paths = Vector<string>();

    for(int i=0; i<getEncodingCount(header); i++) {
        keys.add(getDecodedChar(header, i));
        paths.add(getDecodedPath(header, i));
    }

    recreateTreeFromHeader(header, root, keys, paths);
}


/**
 * @brief recreateTreeFromHeader calls recursive method to recreate the tree from its encoding
 * @param encoding
 * @return
 */
HuffmanNode* recreateTreeFromHeader(string encoding)
{
    HuffmanNode* root = new HuffmanNode(nullptr, nullptr);

    recreateTreeFromHeader(encoding, root);

    return root;
}


/**
 * @brief freeTree deletes all the nodes allocated as part of a tree
 * @param t
 */
void freeTree(HuffmanNode* root)
{
    if(root == nullptr) {
        return;
    }

    freeTree(root->zero);
    freeTree(root->one);

    delete root;
}


/**
 * @brief getBit converts the character at a position in a string into a 0 or 1 bit int
 * @param txt
 * @param position
 * @return
 */
int getBit(string txt, int position) {
    return stoi(txt.substr(position, 1));
}


/**
 * @brief compressLine compresses a line relying on an encoding map, and places it in an output file
 * @param line
 * @param encodingMap
 * @param output
 */
void compressLine(string line, Map<char, string>& encodingMap, HuffmanOutputFile& output) {
    for(int i=0; i<line.size(); i++) {
        char ch = line[i];
        string encoded = encodingMap.get(ch);  // a code like "010"
        for(int j=0; j<encoded.size(); j++) {
            int bit = getBit(encoded, j);
            output.writeBit(bit);
        }
    }
}


/**
 * @brief compressFile compress the input, place it in the otuput file, header first
 * @param input
 * @param header
 * @param encodingMap
 * @param output
 */
void compressFile(istream& input, string header, Map<char, string>& encodingMap, HuffmanOutputFile& output) {

    output.writeHeader(header + "\n");

    string line;
    getline(input, line);
    while (input) {
        compressLine(line+"\n", encodingMap, output);
        getline(input, line);
    }
}


/**
 * @brief compress compresses an input string and places it with its header in an output file
 * @param input
 * @param output
 */
void compress(istream& input, HuffmanOutputFile& output)
{
    // table
    Map<char, int> freqTable = buildFrequencyTable(input);

    // tree
    HuffmanNode* encodingTree = buildEncodingTree(freqTable);

    // map
    Map<char, string> encodingMap = buildEncodingMap(encodingTree);
    // cout << "encodingMapBefore=" << encodingMap << endl << endl;

    // header
    string header = flattenTreeToHeader(encodingTree);
    // cout << "headerBefore=" << header << endl << endl;

    // must rewind the stream to the beginning in order to read it a second time
    input.clear();                  // removes any current eof/failure flags
    input.seekg(0, std::ios::beg);  // tells the stream to seek back to the beginning

    compressFile(input, header, encodingMap, output);

    // free
    freeTree(encodingTree);
}


/**
 * @brief getValidKey from an encoding map, it determines the character key corresponding to a string value
 * @param encodingValue
 * @param encodingMap
 * @return
 */
char getValidKey(string encodingValue, Map<char, string>& encodingMap) {

    char validKey = '*';

    for(char encodingKey:encodingMap.keys()) {
        string keyValue = encodingMap.get(encodingKey);
        if(encodingValue == keyValue) {
            validKey = encodingKey;
        }
    }

    return validKey;
}


/**
 * @brief getValidKey caches character coresponding to a path
 * Example: for (g.10111), then 10111->g
 * @param encodingValue
 * @param encodingMap
 * @param charCache
 * @return
 */
char getValidKey(string encodingValue, Map<char, string>& encodingMap, Map<string, char>& charCache) {

    if(charCache.containsKey(encodingValue)) {
        return charCache.get(encodingValue);
    }

    char actual = getValidKey(encodingValue, encodingMap);
    charCache.put(encodingValue, actual);
    return actual;
}

/**
 * @brief writeBit writes a bit to a file
 * @param ch
 * @param output
 */
void writeBit(char ch, ostream& output) {
    int size = 1;

    // allocate memory for file content
    char* buffer = new char[size];
    buffer[0] = ch;

    // write to outfile
    output.write(buffer, size);
}


/**
 * @brief isValidValueL2 determines if a value is contained in a map
 * @param encodingValue
 * @param encodingMap
 * @return
 */
bool isValidValueL2(string encodingValue, Map<char, string>& encodingMap) {

    for(char encodingKey:encodingMap.keys()) {
        string keyValue = encodingMap.get(encodingKey);
        if(encodingValue == keyValue) {
            return true;
        }
    }

    return false;
}


/**
 * @brief isValidValueL1 caches the result of checking if isValidValueL2
 * @param encodingValue
 * @param encodingMap
 * @return
 */
bool isValidValueL1(string encodingValue, Map<char, string>& encodingMap, Map<string, bool>& isValidCache) {

    if(isValidCache.containsKey(encodingValue)) { // cached
        return isValidCache.get(encodingValue);
    }

    bool actual = isValidValueL2(encodingValue, encodingMap);
    isValidCache.put(encodingValue, actual);

    return actual;
}


/**
 * @brief decompressFile decompresses an input file into an output file
 * @param input
 * @param encodingMap
 * @param output
 */
void decompressFile(HuffmanInputFile& input, Map<char, string>& encodingMap, ostream& output) {

    string encodedChar = "";

    Map<string, bool> isValidCache = Map<string, bool>();
    Map<string, char> charCache = Map<string, char>();

    int bit = input.readBit();
    while (bit != -1) {

        encodedChar += to_string(bit);

        if(isValidValueL1(encodedChar, encodingMap, isValidCache)) {
            char ch = getValidKey(encodedChar, encodingMap, charCache);
            writeBit(ch, output);
            encodedChar = ""; // reset
        }

        bit = input.readBit();
    }
}


/**
 * @brief decompress input -> decompress -> output
 * @param input
 * @param output
 */
void decompress(HuffmanInputFile& input, ostream& output)
{
    // header
    string encoding = input.readHeader();
    string cleanEncoding = encoding.substr(0, encoding.size()-1); // removes '\n' at end of line
    // cout << "headerAfter=" << cleanEncoding << endl << endl;

    // tree
    HuffmanNode* encodingTree = recreateTreeFromHeader(cleanEncoding);

    // map
    Map<char, string> encodingMap = buildEncodingMap(encodingTree);
    // cout << "encodingMapAfter=" << encodingMap << endl << endl;

    decompressFile(input, encodingMap, output);

    // free
    freeTree(encodingTree);
}
