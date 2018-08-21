#include <iostream>
#include <vector>
#include <map>
#include <fstream>

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
    // which node and which is the next char that should come for the algorithm to continue walk on this edge
    struct NodePointer {
        char charOnEdge;
        Node *node;
        int edgeOffset;

        NodePointer(char _charOnEdge, Node *_node) : charOnEdge(_charOnEdge), node(_node), edgeOffset(0) {};
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
        //first function
        //findString(c, root, matchSubstitution);

        //second function
        vector<NodePointer> listOfCandidates;
        listOfCandidates.emplace_back(NodePointer('a', root));
        findStringWithPointers2(c, listOfCandidates, text);
    }

    void findString(char *currentCharInString, Node *n, string text) {
        int edgeOffset = 0;
        for (auto const &node: n->children){
            //offset is for edge
            char currentCharOnEdge = text.at(node.second->first - 1 + edgeOffset);
            if(*currentCharInString == currentCharOnEdge){
                cout << "same";
                currentCharInString++;
                //if all chars on the edge are examined go to the child node
                if(node.second->first - 1 + edgeOffset == *node.second->last - 1){
                    findString(currentCharInString, node.second, text);
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
                findString(currentCharInString, root, text);
            }
        }
    }

    void findStringWithPointers2(char *currentCharInString, vector<NodePointer> listOfPointers, string text){
        int length = 0;
        while(length <= text.length()){
            for(int i = 0; i < listOfPointers.size(); i++) {
                //char *charInStringForPointer = currentCharInString
                bool found = false;
                for (auto const &node: listOfPointers[i].node->children){
                    if(!(*currentCharInString == node.first)) {
                        continue;
                    }
                    listOfPointers[i].charOnEdge = text.at(node.second->first - 1 + listOfPointers[i].edgeOffset);

                    if(*currentCharInString == listOfPointers[i].charOnEdge){
                        found = true;
                        //charInStringForPointer++;
                        //needs to go to child of the node, examined all chars on the edge
                        if(node.second->first - 1 + listOfPointers[i].edgeOffset == *node.second->last - 1){
                            listOfPointers[i].node = node.second;
                            listOfPointers[i].edgeOffset = 0;
                            break;
                        }
                        //not all chars from the edge examined
                        else {
                            listOfPointers[i].edgeOffset++;
                            break;
                        }
                    } else {
                        //if(currentCharInString == charInStringForPointer)
                            //listOfPointers.emplace_back(NodePointer(listOfPointers[i].charOnEdge, root));
                        break;
                    }
                }
                if(!found)
                    listOfPointers.emplace_back(NodePointer(listOfPointers[i].charOnEdge, root));
            }

            length++;
            if(length <= text.size()){
                currentCharInString++;
            }
        }
    }

    void findStringWithPointers(char *currentCharInString, string text, vector<NodePointer> listOfCandidates) {
        for(int i = 0; i < listOfCandidates.size(); i++){
            listOfCandidates[i].edgeOffset = 0;
            for (auto const &node: listOfCandidates[i].node->children){
                //offset is for edge
                listOfCandidates[i].charOnEdge = text.at(node.second->first - 1 + listOfCandidates[i].edgeOffset);
                //listOfCandidates[i].waitingForChar = text.at(node.second->first + edgeOffset);
                //found char on the edge
                if(*currentCharInString == listOfCandidates[i].charOnEdge){
                    cout << "same";
                    //if all chars on the edge are examined go to the child node
                    if(node.second->first - 1 + listOfCandidates[i].edgeOffset == *node.second->last - 1){
                        listOfCandidates[i].node = node.second;
                        i = -1;
                        break;
                        //currentCharInString++;
                        //findStringWithPointers(currentCharInString, text, listOfCandidates);
                    }
                    //if not, continue examining char on the edge
                    else {
                        listOfCandidates[i].edgeOffset = listOfCandidates[i].edgeOffset + 1;
                        continue;
                    }
                }
                else {
                    cout << "dif";
                    //listOfCandidates.emplace_back(NodePointer(listOfCandidates[i].charOnEdge, root, listOfCandidates[i].waitingForChar));
                    //currentCharInString++;
                    i = -1;
                    break;
                    //findStringWithPointers(currentCharInString, text, listOfCandidates);
                }
            }
            currentCharInString++;
        }
        findStringWithPointers(currentCharInString, text, listOfCandidates);

    }

};

string notes = "abcabaxabcd";

string notes2 = "d3.c3#c3#b2.b2.c3#d3.c3#b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.e3.f3#e3.e3.f3#e3.d3.d3.e3.f3#e3.a3.a3.d3.b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.c3#c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.e3.d3.c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.c3#c3#b2.d3.c3#d3.d2.d3.c3#d3.c2#e3.d3.e3.d3.c3#b2.f2#g2.f2#a2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.d2.f2#a2.d2.f2#g2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.b2.a2.g2.f2#e2.e2.d2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.b2.a2.g2.f2#g2.a2.d2.f2#a2.d2.f2#g2.f2#g2.f2#g2.a2.f2#g2.f2#g2.a2.b2.a2.g2.f2#e2.d2.f2#g2.f2#e2.d2.f2#a2.d2.f2#a2.d2.f2#g2.f2#e2.e2.d2.f2#g2.f2#e2.a1.d2.a2.a1.d2.a2.a1.d2.g2.a1.d2.g2.a1.d2.f2#a1.d2.f2#a1.d2.g2.a1.d2.d3.d2.a2.d3.d2.a2.d3.d2.g2.d3.d2.g2.d3.d2.f2#d3.d2.f2#d3.d2.g2.d3.d2.g2.d3.f2#a2.d3.f2#a2.d3.e2.g2.d3.e2.g2.d3.d2.f2#d3.d2.f2#d3.d2.g2.d3.d2.g2.d3.f2#d3.f2#d3.f2#d3.e2.d3.e2.d3.e2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.d2.d3.f3#b2.g3.a3.c3#d3.b3.f3#e3.a3.c3#d3.b3.f3#e3.d3.c3#a2.f3#b2.g3.a3.c3#d3.b2.b2.d2.d3.c3#c3#b2.b2.c3#d3.c3#b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.e3.f3#e3.e3.f3#e3.d3.d3.e3.f3#e3.a3.a3.d3.b2.c3#b2.c3#d3.c3#c3#d3.c3#d3.c3#b2.b2.c3#d3.c3#e3.d3.c3#b2.d3.c3#d3.d3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.a3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.b3.a3.g3.f3#e3.e3.d3.d3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.b3.a3.g3.f3#e3.f3#a3.a3.f3#g3.f3#g3.a3.f3#g3.f3#g3.a3.b3.a3.g3.f3#e3.d3.d3.f3#g3.f3#e3.d3.f3#a3.a3.f3#g3.f3#e3.d3.d3.f3#g3.f3#e3.a3.a2.d3.a3.a2.d3.a3.a2.d3.g3.a2.d3.g3.a2.d3.f3#a2.d3.f3#g2.a2.d3.g2.a2.a3.a2.d3.a3.a2.d3.a3.a2.d3.g3.a2.d3.g3.a2.d3.f3#a2.d3.f3#a2.d3.g3.a2.d3.d4.d3.a3.d4.d3.g3.d4.d3.g3.d4.d3.f3#d4.d3.f3#d4.d3.g3.d4.d3.g3.d4.d3.a3.d4.d3.a3.d4.d3.g3.d4.d3.g3.d4.d3.f3#d4.d3.f3#d4.d3.g3.d4.d3.g3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d4.d3.d3.";

int main() {
    cout << "Hello, World!" << endl;
    SuffixTree sf;
    if( remove( ".././output.txt" ) != 0 )
        perror( "Error deleting file" );
    else
        puts( "File successfully deleted" );
    sf.buildTree(notes);


    return 0;
}