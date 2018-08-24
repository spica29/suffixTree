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

    void buildTree(string text) {
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

        string matchSubstitution = "abbabaxabcd";
        cout << "size " << matchSubstitution.length();
        char *c = &matchSubstitution.at(0);
        //vector<NodeTable> nodeTable;
        map<int, float> frequencyMap;
        //first function
        //findString(c, root, matchSubstitution, frequencyMap, 1);

        //second function
        vector<NodePointer> listOfCandidates;
        listOfCandidates.emplace_back(NodePointer('a', root));
        findStringWithPointers(c, listOfCandidates, text);
    }

    void findString(char *currentCharInString, Node *n, string text, vector<pair<int, float> > &frequencyPairs, int position, int offsetFromRoot) {
        int edgeOffset = 0;
        for (auto const &node: n->children){
            //offset is for edge
            char currentCharOnEdge = text.at(node.second->first - 1 + edgeOffset);
            if(*currentCharInString == currentCharOnEdge){
                //increase values of pair's position (first) by 1
                int i = 0;
                for(auto &frequencyPair: frequencyPairs){
                    frequencyPair.first = frequencyPair.first + 1;
                    if(frequencyPair.first == n->suffixIndices[i] + offsetFromRoot){
                        //frequencyPair.first = n->suffixIndices[i] ;
                        frequencyPair.second++;
                    }
                    i++;
                }


                cout << "same";
                currentCharInString++;
                //if all chars on the edge are examined go to the child node
                if(node.second->first - 1 + edgeOffset == *node.second->last - 1){
                    findString(currentCharInString, node.second, text, frequencyPairs, position + 1, offsetFromRoot + 1);
                }
                //if not, continue examining char on the edge
                else {
                    edgeOffset++;
                    continue;
                }
            }
            else {
                cout << "dif";
                currentCharInString++;
                findString(currentCharInString, root, text, frequencyPairs, position + 1, offsetFromRoot + 1);
            }
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
        pattern.insert(randNum, 1, 'k');
    }

    cout << pattern;
    return pattern;
}

string changeStringSubstitution(string pattern, int k){
    srand((int)time(0));
    for (int i = 0; i < k; i++) {
        int randNum = rand() % pattern.size();
        pattern.replace(randNum, 1, "k");
    }

    cout << pattern;
    return pattern;
}

string changeStringDeletion(string pattern, int k){
    srand((int)time(0));
    for (int i = 0; i < k; i++) {
        int randNum = rand() % pattern.size();
        pattern.erase(randNum, 1);
    }

    cout << pattern;
    return pattern;
}

void mapScoreToString(string score){
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

    cout << scoreShorten;
}

string notes = "abcabaxabcd";

string notes2 = "d3.c3#c3#b2.b2.c3#d3.c3#b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.e3.f3#e3.e3.f3#e3.d3.d3.e3.f3#e3.a3.a3.d3.b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.c3#c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.e3.d3.c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.c3#c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.e3.d3.c3#b2.f2#g2.f2#a2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.d2.f2#a2.d2.f2#g2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.b2.a2.g2.f2#e2.e2.d2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.b2.a2.g2.f2#g2.a2.d2.f2#a2.d2.f2#g2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.b2.a2.g2.f2#e2.d2.f2#g2.f2#e2.d2.f2#a2.d2.f2#a2.d2.f2#g2.f2#e2.e2.d2.f2#g2.f2#e2.a1.d2.a2.a1.d2.a2.a1.d2.g2.a1.d2.g2.a1.d2.f2#a1.d2.f2#a1.d2.g2.a1.d2.d3.d2.a2.d3.d2.a2.d3.d2.g2.d3.d2.g2.d3.d2.f2#d3.d2.f2#d3.d2.g2.d3.d2.g2.d3.f2#a2.d3.f2#a2.d3.e2.g2.d3.e2.g2.d3.d2.f2#d3.d2.f2#d3.d2.g2.d3.d2.g2.d3.f2#d3.f2#d3.f2#d3.e2.d3.e2.d3.e2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.f3#b2.g3.a3.c3#d3.b3.f3#e3.a3.c3#d3.b3.f3#e3.d3.c3#a2.f3#b2.g3.a3.c3#d3.b2.b2.d2.d3.c3#c3#b2.b2.c3#d3.c3#b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.e3.f3#e3.e3.f3#e3.d3.d3.e3.f3#e3.a3.a3.d3.b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.c3#d3.d3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.a3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.b3.a3.g3.f3#e3.e3.d3.d3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.b3.a3.g3.f3#e3.f3#a3.a3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.b3.a3.g3.f3#e3.d3.d3.f3#g3.f3#e3.d3.f3#a3.a3.f3#g3.f3#e3.d3.d3.f3#g3.f3#e3.a3.a2.d3.a3.a2.d3.a3.a2.d3.g3.a2.d3.g3.a2.d3.f3#a2.d3.f3#g2.a2.d3.g2.a2.a3.a2.d3.a3.a2.d3.a3.a2.d3.g3.a2.d3.g3.a2.d3.f3#a2.d3.f3#a2.d3.g3.a2.d3.d4.d3.a3.d4.d3.g3.d4.d3.g3.d4.d3.f3#d4.d3.f3#d4.d3.g3.d4.d3.g3.d4.d3.a3.d4.d3.a3.d4.d3.g3.d4.d3.g3.d4.d3.f3#d4.d3.f3#d4.d3.g3.d4.d3.g3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d3.";

int main() {
    cout << "Hello, World!" << endl;
    /*
     * SuffixTree sf;
    if( remove( ".././output.txt" ) != 0 )
        perror( "Error deleting file" );
    else
        puts( "File successfully deleted" );
    sf.buildTree(notes);*/

    //changeStringDeletion(notes, 3);

    mapScoreToString(notes2);

    return 0;
}