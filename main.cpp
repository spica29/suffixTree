#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace std;

class SuffixTree {
    struct Node {
        //list of children
        map<char, Node*> children;
        Node* parent;
        int number;

        //pointer to other node via suffix link
        Node *suffixLink;
        //start and end index of edge characters
        int first;
        int *last;

        // This will be non-negative for leaves and will give index of suffix for the path from root to this leaf.
        // For non-leaf node, it will be -1.
        int suffixIndex;
        //save list of suffixIndexes of node's subtree
        vector<int> suffixIndices;

        Node() : children(map<char, Node*>()), suffixLink(0), first(0), last(0), parent(0), suffixIndex(-1), suffixIndices(vector<int>()) {};
        Node(map<char, Node*> _children, Node *_suffixLink, int _first, int *_last, int _suffixIndex)
        : children(_children), suffixLink(_suffixLink), first(_first), last(_last), parent(0), suffixIndex(_suffixIndex), suffixIndices(vector<int>()) {};
        Node(map<char, Node*> _children, Node *_suffixLink, Node* _parent, int _number, int _first, int *_last, int _suffixIndex)
        : children(_children), suffixLink(_suffixLink), parent(_parent), number(_number), first(_first), last(_last), suffixIndex(_suffixIndex), suffixIndices(vector<int>()) {};
    };

    //structure for string search which will have information about until which char on the edge algorithm came,
    // which node and the offset of the character at the edge
    struct NodePointer {
        char charOnEdge;
        Node *node;
        int edgeOffset;

        NodePointer(char _charOnEdge, Node *_node) : charOnEdge(_charOnEdge), node(_node), edgeOffset(0) {};

        bool operator==(const NodePointer& np1){
            return (np1.node == node && np1.edgeOffset == edgeOffset);
        }
        bool operator!=(const NodePointer& np1){
            return !(np1.node == node && np1.edgeOffset == edgeOffset);
        }
    };

    struct NodeTable {
        int position;
        float frequency;
        map<int, float> frequencyMap;

        NodeTable(int _position, int _frequency) : position(_position), frequency(_frequency) {};
        NodeTable() : position(0), frequency(0) {};
    };

    Node *root; //pointer to root node
    Node *internalNode;
    //activePoint
    Node *activeNode;
    int activeEdge;
    int activeLength;

    int *splitEnd;
    int *rootEnd;
    //remainingSuffixCount - how many suffixed to be add in tree (explicitly)
    int remainingSuffixCount;
    int *END;

public:
    SuffixTree() : internalNode(nullptr), activeNode(root), activeEdge(-1), activeLength(0), remainingSuffixCount(0), END(0) {}
    void extension(int *position, string text, int *suffixIndex, int *counter) {
        //extension rule 1
        this->END = position;
        //increase remainingSuffixCount
        remainingSuffixCount++;
        //when starting new phase no internal node is waiting for it's suffix link update in current phase
        internalNode = nullptr;

        while(remainingSuffixCount > 0) {
            //APCFALZ
            if(activeLength == 0){
                activeEdge = *position; //position of character in string
            }

            //there does not exist edge going out from activeNode starting with activeEdge
            if(activeNode->children.find(text[activeEdge - 1]) == activeNode->children.end()) {
                //extension rule 2
                //create new leaf edge
                activeNode->children[text[activeEdge - 1]] = new Node(map<char, Node*>(), root, activeNode, *counter, *position, END, *suffixIndex);
                Node *newLeaf = activeNode->children[text[activeEdge - 1]];
                //update suffixIndices for parents of new node
                newLeaf->suffixIndices.emplace_back(*suffixIndex);
                while(newLeaf->parent != NULL){
                    newLeaf->parent->suffixIndices.emplace_back(*suffixIndex);
                    newLeaf = newLeaf->parent;
                }
                (*suffixIndex)++;
                (*counter)++;
                //check suffix Link
                if(internalNode != nullptr){
                    internalNode->suffixLink = activeNode;
                    internalNode = nullptr;
                }
            }
            // edge exists
            else {
                Node *next = activeNode->children[text[activeEdge - 1]];
                //walk down
                if(walkDown(*next)){
                    continue;
                }

                //extension rule 3 - current char already on the edge
                if(text[next->first + activeLength - 1] == text[*position - 1]){
                    if(internalNode != nullptr && activeNode != root) {
                        internalNode->suffixLink = activeNode;
                        internalNode = nullptr;
                    }

                    //APCFER3
                    activeLength++;
                    //move to next phase
                    break;
                }

                //adding new internal node and new leaf edge (extension rule 2)
                splitEnd = new int(next->first + activeLength - 1);

                //new internal node
                Node *split = new Node(map<char, Node*>(), root, next->parent, *counter, next->first, splitEnd, -1);
                (*counter)++;
                split->suffixIndices.insert(split->suffixIndices.begin(), next->suffixIndices.begin(), next->suffixIndices.end());
                activeNode->children[text[activeEdge - 1]] = split;

                //adding leaf out from internal node
                split->children[text[*position - 1]] = new Node(map<char, Node*>(), root, split, *counter, *position, END, *suffixIndex);
                Node *newLeaf = split->children[text[*position - 1]];
                newLeaf->suffixIndices.emplace_back(*suffixIndex);
                next->first += activeLength;
                next->parent = split;
                (*counter)++;
                (*suffixIndex)++;
                split->children[text[*split->last]] = next;
                //update suffixIndices for parents of new node

                while(newLeaf->parent != NULL){
                    newLeaf->parent->suffixIndices.emplace_back(*suffixIndex - 1);
                    newLeaf = newLeaf->parent;
                }
                //add suffix link
                if(internalNode != nullptr){
                    internalNode->suffixLink = split;
                }

                internalNode = split;
            }

            //suffix added to the tree
            remainingSuffixCount--;
            if (activeNode == root && activeLength > 0){ //APCFER2C1
                activeLength--;
                activeEdge = *position - remainingSuffixCount + 1;
            }
            else if (activeNode != root) { //APCFER2C2
                activeNode = activeNode->suffixLink;
            }
        }
    }
    int edgeLength(Node *node) { return *node->last - node->first + 1; }
    bool walkDown(Node &node) {
        int edgeLengthValue = edgeLength(&node);
        if (activeLength >= edgeLengthValue){
            activeEdge += edgeLengthValue;
            activeLength -= edgeLengthValue;
            activeNode = &node;
            return true;
        }
        return false;
    }

    void printGraphvizDFS(Node *n, string text){
        if(n == NULL) return;
        //check suffix link
        /*
        if(n->suffixLink != NULL && n->suffixLink != root) {
            std::fstream file;
            file.open (".././output.txt", std::fstream::in | std::fstream::out | std::fstream::app);

            if (!file) { std::cerr<<"Error writing to ..."<<std::endl; }
            else
                file  << n->number << " -- " << n->suffixLink->number << " [ color = \"blue\" ];" << endl;
            file.close();
        }
         */

        for (auto const &node: n->children){
            std::fstream file;
            file.open (".././output.txt", std::fstream::in | std::fstream::out | std::fstream::app);

            if (!file) { std::cerr<<"Error writing to ..."<<std::endl; }
            else{
                file  << n->number << " -- " << node.second->number << " [ label=\"" + text.substr(node.second->first - 1, *node.second->last - node.second->first + 1) + "\", xlabel=\"";
                for (int i = 0; i < node.second->suffixIndices.size(); i++) {
                    file << node.second->suffixIndices[i] << " ";
                }

                file << "\" ];" << endl;
            }
            file.close();

            printGraphvizDFS(node.second, text);
        }
    }

    void writeInFile(string text, string name){
        std::fstream file;
        file.open (name, std::fstream::in | std::fstream::out | std::fstream::app);

        if (!file) { std::cerr<<"Error writing to ..."<<std::endl; }
        else{
            file << text << endl;
        }
        file.close();
    }

    void printTreeDFS(Node *n, string text)
    {
        if (n == NULL)  return;
        cout << "first: " << n->first << ", last: " <<  *n->last << ", suffix index: " << n->suffixIndex << endl;
        for (int i = 0; i < text.size(); i++){
            if(n->children[text[i]] == nullptr) continue;
            //if(!(text[i] > n->first) || n->first == 0)
            bool char_repeated = false;
            //check if node is already printed, char like this happened before! (repetitions)
            for (int j = 0; j < i; j++) {
                if(text[i] == text[j])
                {
                    char_repeated = true;
                    break;
                }
            }
            if(!char_repeated)
            printTreeDFS(n->children[text[i]], text);
        }
    }

    void freeSuffixTreeByPostOrder(Node *n, string text)
    {
        if (n == NULL)
            return;
        int i;
        for (i = 0; i < text.size(); i++)
        {
            if (&n->children[text[i]] != NULL)
            {
                freeSuffixTreeByPostOrder(n->children[text[i]], text);
            }
        }
        if (n->suffixIndex == -1)
            delete(&n->last);
        delete(n);
    }

    void buildTree(string text, string pattern) {
        rootEnd = new int(0);
        root = new Node();
        root->first = 0;
        root->number = 0;
        root->last = rootEnd;

        activeNode = root;
        int *suffixIndex = new int(1);
        int *counter = new int(1);
        int *i = new int(1);
        for (*i = 1; *i <= text.size(); ++*i) {
            extension(i, text, suffixIndex, counter);
        }
        *i = text.size();
        //printTreeDFS(root, text);
        char *first = new char('a');
        char *child = new char('b');
        int *first_num = new int(0);
        int *child_num = new int(1);
        printGraphvizDFS(root, text);

        //string matchSubstitution = "abdabaxabcd";
        //cout << "pattern:" << matchSubstitution << ", size of pattern: " << matchSubstitution.length() << endl;
        char *c = &pattern.at(0);
        //vector<NodeTable> nodeTable;
        vector<pair <int, float> >frequencyPair;

        //first function
        findStringFirst(c, root, 0, pattern, text);
        writeInFile("\n", ".././results.txt");
        findStringSecond(c, root, 0, pattern, frequencyPair, text);


        //second function
        //vector<NodePointer> listOfCandidates;
        //listOfCandidates.emplace_back(NodePointer('a', root));
        //findStringWithPointers(c, listOfCandidates, text);
    }

    void addToTheTable(Node *n, vector<pair<int, float> > &frequencyPairs, int offsetFromRoot, string text, int numberOfCorrect, int &lastMax){
        //increase values of pair's position (first) by 1
        pair<int, float> *max = nullptr;
        if(frequencyPairs.size() > 0){
            max = &frequencyPairs[0];
        }
        bool added = false;
        vector<int> suffixIndicesCopy;
        suffixIndicesCopy = n->suffixIndices;
        //int i = 0;
        //for(auto &frequencyPair: frequencyPairs){
        for(int i = 0; i < frequencyPairs.size(); i++){
            //erase pair with position bigger than size
            /*
            if(frequencyPair.first + 1 > text.size()){
                frequencyPairs.erase(frequencyPairs.begin() + i);
                continue;
            }*/
            //check if returned to the root/mistake happened
            //if true add one more element which is the last maximum in the iteration before coming back to the root
            if(offsetFromRoot == 0 && frequencyPairs.size() > 1 && frequencyPairs[i].first == lastMax && i == 0){
                pair<int, float> newPair = make_pair(frequencyPairs[i].first - 1, frequencyPairs[i].second);
                pair<int, float> newPair2 = make_pair(frequencyPairs[i].first + 1, frequencyPairs[i].second);
                frequencyPairs.insert(frequencyPairs.end(), newPair2);
                frequencyPairs.insert(frequencyPairs.end(), newPair);
                //max = &frequencyPair;
                //cout << "size " << frequencyPairs.size();
                added = true;
            }

            if(added){
                if(frequencyPairs[i].first == frequencyPairs[frequencyPairs.size() - 1].first && i != frequencyPairs.size() - 1){
                    frequencyPairs.erase(frequencyPairs.begin() + i);
                }
                if(frequencyPairs[i].first == frequencyPairs[frequencyPairs.size() - 2].first && i != frequencyPairs.size() - 2){
                    frequencyPairs.erase(frequencyPairs.begin() + i);
                }
            }

            frequencyPairs[i].first = frequencyPairs[i].first + 1;
            if(added && i == 0 ) frequencyPairs[i].second += 1;
            //check suffix index to increase frequency value
            for(int j = 0; j < suffixIndicesCopy.size(); j++){
                if(frequencyPairs[i].first == suffixIndicesCopy[j] + offsetFromRoot){
                    //frequencyPair.first = n->suffixIndices[i];
                    suffixIndicesCopy.erase(suffixIndicesCopy.begin() + j);
                    //if(added && i == 0 ) frequencyPair.second += 2;
                    //else
                        frequencyPairs[i].second += 1;
                    break;
                }
            }

            //remember the element with maximum frequency
            if((frequencyPairs[i].second > max->second ||(frequencyPairs[i].second == max->second && frequencyPairs[i].first < max->first))){
                //if(added && i == 0) continue;
                max = &frequencyPairs[i];
                /*
                if(i == frequencyPairs.size() - 2){
                    if((frequencyPairs[frequencyPairs.size() - 1].second > max->second ||(frequencyPairs[frequencyPairs.size() - 1].second == max->second && frequencyPairs[frequencyPairs.size() - 1].first < max->first))){
                        max = &frequencyPairs[frequencyPairs.size() - 1];
                    }
                    if((frequencyPairs[frequencyPairs.size() - 2].second > max->second ||(frequencyPairs[frequencyPairs.size() - 2].second == max->second && frequencyPairs[frequencyPairs.size() - 2].first < max->first))){
                        max = &frequencyPairs[frequencyPairs.size() - 2];
                    }
                }
                 */
            }
            //i++;
        }
        //move max to the first place
        if(max != nullptr && max != &frequencyPairs[0]){
            iter_swap(frequencyPairs.begin(), max);
        }

        //not in the vector, need to be added
        for(int i = 0; i < suffixIndicesCopy.size(); i++){
            frequencyPairs.emplace_back(suffixIndicesCopy[i], 1);
        }
        lastMax = frequencyPairs[0].first;
    }

    void findStringFirst(char *currentCharInPattern, Node *n, Node *child, string pattern, string text, int position = 1, int edgeOffset = 0, int offsetFromRoot = 0, int suffixIndexOfEdge = -1, int numberOfCorrect = 0, int min_pos = -1) {
        bool found = false;
        if(pattern[pattern.size()] == *currentCharInPattern) return;
        for (auto &node: n->children){
            //offset is for edge
            if(node.second->first == text.size() || (node.second->first - 1 + edgeOffset >= text.size())) continue;
            char currentCharOnEdge = text.at(node.second->first - 1 + edgeOffset);
            if(*currentCharInPattern != currentCharOnEdge || (node.second != child && child != 0)) {
                //pointer already showing on the char on the edge, don't check neighbours
                if(edgeOffset != 0 && node.second == child){
                    break;
                }
                continue;
            }
            /*
            if(find(node.second->suffixIndices.begin(), node.second->suffixIndices.end(), suffixIndexOfEdge) == node.second->suffixIndices.end() && suffixIndexOfEdge != -1){
                //position--; //skip the char
                //numberOfCorrect--;
                continue;
            }
            */
            else {
                int suffix = -1;
                //take the smallest suffix index bigger than position
                for(int i = 0; i < node.second->suffixIndices.size(); i++){
                    suffix = node.second->suffixIndices[i];
                    int currentPosition = suffix + offsetFromRoot;
                    if(numberOfCorrect <= currentPosition){
                        min_pos = currentPosition;
                        break;
                    } else if (i == node.second->suffixIndices.size()-1) {
                        min_pos = currentPosition;
                    }
                }
                //if(*currentCharInPattern == currentCharOnEdge) {
                    //if(min_pos == -1) //from this char can't be continued, but will still try to predict

                found = true;
                numberOfCorrect++;
                cout << " char in pattern: " << *currentCharInPattern << " char in pattern position: " << position
                     << ", predicted char position: " << min_pos << endl;
                //cout <<"next char in pattern position: " << position + 1 << ", predicted char position: " << min_pos << endl;
                //int difference = min_pos - position;
                string fullname = ".././results.txt";
                writeInFile(std::to_string(min_pos), fullname);
                //cout << " char in pattern: " << *currentCharInPattern << ", predicted char: " << text[node.second->first + edgeOffset] << endl;
                currentCharInPattern++;
                //if all chars on the edge are examined go to the child node
                if (node.second->first - 1 + edgeOffset == *node.second->last - 1) {
                    findStringFirst(currentCharInPattern, node.second, 0, pattern, text, position + 1, 0,
                                    offsetFromRoot + 1, suffix, numberOfCorrect, min_pos);
                    break;
                }
                    //if not, continue examining char on the edge
                else {
                    edgeOffset++;
                    findStringFirst(currentCharInPattern, n, node.second, pattern, text, position + 1, edgeOffset,
                                    offsetFromRoot + 1, suffix, numberOfCorrect, min_pos);
                    break;
                    //}
                }
            }
        }

        //check case when char from pattern is not in the alphabet
        if(!found){
            if(n == root && edgeOffset == 0){
                if(min_pos == -1)
                    min_pos = 1;
                else min_pos = min_pos + 1;
                cout << " char in pattern: " << *currentCharInPattern << " char in pattern position: " << position << ", predicted char position: " << min_pos << endl;
                //cout <<"next char in pattern position: " << position + 1 << ", predicted char position: " << min_pos << endl;
                int difference = min_pos - position;
                string fullname = ".././results.txt";
                writeInFile(std::to_string(difference), fullname);
                currentCharInPattern++;
                position++;
            }
            findStringFirst(currentCharInPattern, root, 0, pattern, text, position, 0, 0, -1, numberOfCorrect, min_pos);
        }
    }

    void findStringSecond(char *currentCharInPattern, Node *n, Node *child, string pattern, vector<pair<int, float> > &frequencyPairs, string text, int position = 1, int offsetFromRoot = 0, int edgeOffset = 0, int numberOfCorrect = 0, int lastMax = -1) {
        bool found = false;
        if(pattern[pattern.size()] == *currentCharInPattern) return;
        for (auto &node: n->children){
            //offset is for edge
            if(node.second->first == text.size() || (node.second->first - 1 + edgeOffset >= text.size())) continue;
            char currentCharOnEdge = text.at(node.second->first - 1 + edgeOffset);
            if(*currentCharInPattern != currentCharOnEdge || (node.second != child && child != 0)) {
                //pointer already showing on the char on the edge, don't check neighbours
                if(edgeOffset != 0 && node.second == child){
                    break;
                }
                continue;
            }
            else {
                found = true;
                numberOfCorrect++;
                addToTheTable(node.second, frequencyPairs, offsetFromRoot, text, numberOfCorrect, lastMax);
                int max = frequencyPairs[0].first;
                //cout << " char in pattern: " << *currentCharInPattern << " char in pattern position: " << position << ", predicted char position: " << max << endl;
                cout <<"next char in pattern position: " << position << ", predicted char position: " << max << endl;
                //int difference = max - position;
                string fullname = ".././results.txt";
                writeInFile(std::to_string(max), fullname);
                //cout << " char in pattern: " << *currentCharInPattern << " max position: " << max << ", predicted char: " << text[max] << endl;
                currentCharInPattern++;
                //if all chars on the edge are examined go to the child node
                if(node.second->first - 1 + edgeOffset == *node.second->last - 1){
                    findStringSecond(currentCharInPattern, node.second, 0, pattern, frequencyPairs, text, position + 1, offsetFromRoot + 1, 0, numberOfCorrect, lastMax);
                    break;
                }
                //if not, continue examining char on the edge
                else {
                    edgeOffset++;
                    child = node.second;
                    findStringSecond(currentCharInPattern, n, child, pattern, frequencyPairs, text, position + 1, offsetFromRoot + 1, edgeOffset, numberOfCorrect, lastMax);
                    break;
                }
            }

        }

        //check case when char from pattern is not in the alphabet
        if(!found){
            if(n == root && edgeOffset == 0){
                int max;
                if(frequencyPairs.size() == 0){
                    max = 0;
                }
                else max = frequencyPairs[0].first;
                cout <<"next char in pattern position: " << position << ", predicted char position: " << max + 1 << endl;
                int difference = max - position;
                string fullname = ".././results.txt";
                writeInFile(std::to_string(difference), fullname);
                currentCharInPattern++;
                position++;
            }
            findStringSecond(currentCharInPattern, root, 0, pattern, frequencyPairs, text, position, 0, 0, numberOfCorrect, lastMax);
        }
    }

    void findStringWithPointers(char *currentCharInString, vector<NodePointer> listOfPointers, string text){
        int length = 0;
        while(length <= text.length()){
            bool found = false;
            for(int i = 0; i < listOfPointers.size(); i++) {
                for (auto const &node: listOfPointers[i].node->children){
                    char charOnTheEdge = text.at(node.second->first - 1 + listOfPointers[i].edgeOffset);
                    if(!(*currentCharInString == charOnTheEdge)) {
                        //pointer already showing on the char on the edge, don't check neighbours
                        if(listOfPointers[i].edgeOffset != 0){
                            break;
                        }
                        continue;
                    }
                    listOfPointers[i].charOnEdge = charOnTheEdge;
                    found = true;

                    //needs to go to child of the node, examined all chars on the edge
                    if(node.second->first - 1 + listOfPointers[i].edgeOffset == *node.second->last - 1){
                        listOfPointers[i].node = node.second;
                        listOfPointers[i].edgeOffset = 0;
                        //check if same NodePointer already exists erase it, no need to duplicate
                        for (int j = 0; j < listOfPointers.size(); j++) {
                            if(listOfPointers[j] == listOfPointers[i] && i != j){
                                listOfPointers.erase(listOfPointers.begin() + j);
                            }
                        }
                        break;
                    }
                    //not all chars from the edge examined
                    else {
                        listOfPointers[i].edgeOffset++;
                        break;
                    }
                }
                //if no children have given char, a mistake happened, start from the root with a new pointer
                //check this only when every NodePointer is searched and char was not found
                if(!found && i == listOfPointers.size() - 1 && listOfPointers[listOfPointers.size()-1].node != root)
                    listOfPointers.emplace_back(NodePointer('a', root));
            }

            length++;
            if(length <= text.size()){
                currentCharInString++;
            }
        }
    }

};

//insert k chars at random positions
string changeStringAddition(string pattern, int k){
    srand((int)time(0));
    for (int i = 0; i < k; i++) {
        int randNum = rand() % pattern.size();
        int randPosForLetter = rand() % pattern.size();
        char letter = pattern[randPosForLetter];
        pattern.insert(randNum, 1, letter);
    }

    cout << "pattern is : " << pattern << endl;
    return pattern;
}

string changeStringSubstitution(string pattern, int k){
    srand((int)time(0));
    for (int i = 0; i < k; i++) {
        int randNum = rand() % pattern.size();
        int randPosForLetter = rand() % pattern.size();
        char letter = pattern[randPosForLetter];
        pattern.replace(randNum, 1, string(1, letter));
    }

    cout << "pattern is : " << pattern << endl;
    return pattern;
}

string changeStringDeletion(string pattern, int k){
    srand((int)time(0));
    for (int i = 0; i < k; i++) {
        int randNum = rand() % pattern.size();
        pattern.erase(randNum, 1);
    }

    cout << "pattern is : " << pattern << endl;
    return pattern;
}

string mapScoreToString(string score){
    map<string, char> mapScore;
    char letter = 'a';
    for (int i = 0; i < score.size(); i = i + 3) {
        string note = score.substr(i, 3);
        //map string of size 3 to one letter
        if(mapScore.find(note) == mapScore.end()){
            mapScore[note] = letter;
            letter++;
        }
    }

    string scoreShorten;
    //use map to make string of scores
    for(int i = 0; i < score.size(); i = i + 3){
        string note = score.substr(i, 3);
        char letter = mapScore.at(note);
        scoreShorten.insert(scoreShorten.size(), 1, letter);
    }
    return scoreShorten;
}

//string notes = "abcabaxabcd";

//string notes2 = "d5.c5#c5#b4.b4.c5#d5.c5#b4.c5#b4.c5#d5.c5#c5#d5.c5#d5.c5#b4.b4.c5#d5.c5#e5.d5.c5#b4.d5.e5.f5#e5.e5.f5#e5.d5.d5.e5.f5#e5.a5.a5.d5.b4.c5#b4.c5#d5.c5#c5#d5.c5#d5.c5#b4.b4.c5#d5.c5#e5.d5.c5#b4.d5.c5#d5.d4.d5.c5#d5.c4#e5.d5.c5#c5#b4.d5.c5#d5.d4.d5.c5#d5.c4#e5.d5.e5.d5.c5#b4.d5.c5#d5.d4.d5.c5#d5.c4#e5.d5.c5#c5#b4.d5.c5#d5.d4.d5.c5#d5.c4#e5.d5.e5.d5.c5#b4.f4#g4.f4#a4.f4#g4.f4#g4.a4.f4#g4.f4#g4.a4.d4.f4#a4.d4.f4#g4.f4#g4.f4#g4.a4.f4#g4.f4#g4.a4.b4.a4.g4.f4#e4.e4.d4.f4#g4.f4#g4.a4.f4#g4.f4#g4.a4.b4.a4.g4.f4#g4.a4.d4.f4#a4.d4.f4#g4.f4#g4.f4#g4.a4.f4#g4.f4#g4.a4.b4.a4.g4.f4#e4.d4.f4#g4.f4#e4.d4.f4#a4.d4.f4#a4.d4.f4#g4.f4#e4.e4.d4.f4#g4.f4#e4.a1.d4.a4.a1.d4.a4.a1.d4.g4.a1.d4.g4.a1.d4.f4#a1.d4.f4#a1.d4.g4.a1.d4.d5.d4.a4.d5.d4.a4.d5.d4.g4.d5.d4.g4.d5.d4.f4#d5.d4.f4#d5.d4.g4.d5.d4.g4.d5.f4#a4.d5.f4#a4.d5.e4.g4.d5.e4.g4.d5.d4.f4#d5.d4.f4#d5.d4.g4.d5.d4.g4.d5.f4#d5.f4#d5.f4#d5.e4.d5.e4.d5.e4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.d4.d5.f5#b4.g5.a5.c5#d5.b5.f5#e5.a5.c5#d5.b5.f5#e5.d5.c5#a4.f5#b4.g5.a5.c5#d5.b4.b4.d4.d5.c5#c5#b4.b4.c5#d5.c5#b4.c5#b4.c5#d5.c5#c5#d5.c5#d5.c5#b4.b4.c5#d5.c5#e5.d5.c5#b4.d5.e5.f5#e5.e5.f5#e5.d5.d5.e5.f5#e5.a5.a5.d5.b4.c5#b4.c5#d5.c5#c5#d5.c5#d5.c5#b4.b4.c5#d5.c5#e5.d5.c5#b4.d5.c5#d5.d5.f5#g5.f5#g5.a5.f5#g5.f5#g5.a5.f5#g5.f5#g5.a5.a5.f5#g5.f5#g5.a5.f5#g5.f5#g5.a5.b5.a5.g5.f5#e5.e5.d5.d5.f5#g5.f5#g5.a5.f5#g5.f5#g5.a5.b5.a5.g5.f5#e5.f5#a5.a5.f5#g5.f5#g5.a5.f5#g5.f5#g5.a5.b5.a5.g5.f5#e5.d5.d5.f5#g5.f5#e5.d5.f5#a5.a5.f5#g5.f5#e5.d5.d5.f5#g5.f5#e5.a5.a4.d5.a5.a4.d5.a5.a4.d5.g5.a4.d5.g5.a4.d5.f5#a4.d5.f5#g4.a4.d5.g4.a4.a5.a4.d5.a5.a4.d5.a5.a4.d5.g5.a4.d5.g5.a4.d5.f5#a4.d5.f5#a4.d5.g5.a4.d5.d6.d5.a5.d6.d5.g5.d6.d5.g5.d6.d5.f5#d6.d5.f5#d6.d5.g5.d6.d5.g5.d6.d5.a5.d6.d5.a5.d6.d5.g5.d6.d5.g5.d6.d5.f5#d6.d5.f5#d6.d5.g5.d6.d5.g5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d6.d5.d5.";

//string notes3 = "d5.c5#c5#b4.b4.c5#d5.c5#b4.c5#b4.c5#d5.c5#c5#d5.c5#d5.c5#b4.b4.c5#d5.c5#e5.d5.c5#b4.d5.e5.f5#e5.e5.f5#e5.d5.d5.e5.f5#e5.a5.a5.d5.b4.c5#b4.c5#d5.c5#c5#d5.c5#d5.c5#b4.b4.c5#d5.c5#e5.d5.c5#";

string notesTwinkle = "c4.c4.g4.g4.a4.a4.g4.f4.f4.e4.e4.d4.d4.c4.g4.g4.f4.f4.e4.e4.d4.g4.g4.f4.f4.e4.e4.d4.c4.c4.g4.g4.a4.a4.g4.f4.f4.e4.e4.d4.d4.c4.";

int main() {
    cout << "Hello, World!" << endl;
    string fullname = ".././results.txt";
    if( remove(fullname.c_str()) != 0 )
        perror( "Error deleting file" );
    else
        puts( "File successfully deleted" );
    string notes = mapScoreToString(notesTwinkle);
    notes.append("$");
    cout << "notes " << notes;
    ///*
    //string pattern = changeStringDeletion(notes, 3);
    string patternIns = "aabbccbddeeffabbdedeefbebddeefadabbccbddeeffa";
    string patternSub = "aabeccbddeefffbbddbefbbddeefaabbccbddeeffa";
    string paternDel = "aabbccbddeeffbbddeefbbddeefaabbccbdeefa";
    //string notes = "abcdef";
    //string pattern = "abccdef";
    cout << endl<< "pattern " << patternIns << endl;

    SuffixTree sf;
    if( remove( ".././output.txt" ) != 0 )
        perror( "Error deleting file" );
    else
        puts( "File successfully deleted" );
    sf.buildTree(notes, patternIns);
     //*/


    //mapScoreToString(notes2);

    return 0;
}