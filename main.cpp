#include <iostream>
#include <vector>
#include <map>

using namespace std;

class SuffixTree {
    struct Node {
        //list of children
        map<char, Node*> children;

        //pointer to other node via suffix link
        Node *suffixLink;
        //start and end index of edge characters
        int first;
        int *last;

        // This will be non-negative for leaves and will give index of suffix for the path from root to this leaf.
        // For non-leaf node, it will be -1.
        int suffixIndex;

        Node() : children(map<char, Node*>()), suffixLink(0), first(0), last(0), suffixIndex(-1) {};
        Node(map<char, Node*> _children, Node *_suffixLink, int _first, int *_last, int _suffixIndex)
        : children(_children), suffixLink(_suffixLink), first(_first), last(_last), suffixIndex(_suffixIndex) {};

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
    void extension(int *position, string text, int *suffixIndex) {
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
                activeNode->children[text[activeEdge - 1]] = new Node(map<char, Node*>(), root, *position, END, *suffixIndex);
                (*suffixIndex)++;
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
                Node *split = new Node(map<char, Node*>(), root, next->first, splitEnd, -1);
                activeNode->children[text[activeEdge - 1]] = split;

                //adding leaf out from internal node
                split->children[text[*position - 1]] = new Node(map<char, Node*>(), root, *position, END, *suffixIndex);
                (*suffixIndex)++;
                next->first += activeLength;
                split->children[text[*split->last]] = next;

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

    //Print the suffix tree as well along with setting suffix index
    //So tree will be printed in DFS manner
    //Each edge along with it's suffix index will be printed
    void printTree(Node *n, string text, int counter)
    {
        if (n == NULL)  return;

        if (n->first != 0) //A non-root node
        {
            //Print the label on edge from parent to current node
            cout << "first: " << n->first << ", last: " <<  *n->last << ", suffix index: " << n->suffixIndex << endl;
        }
        for (int i = 0; i < text.size(); i++)
        {
            if(n->children[text[i]] == nullptr || i < counter) continue;
            if (i < n->children.size())
            {
                //Current node is not a leaf as it has outgoing
                //edges from it.
                setSuffixIndexByDFS(n->children[text[i]], text, i);
            }
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
        root->last = rootEnd;

        activeNode = root;
        int *suffixIndex = new int(1);
        int *i = new int(1);
        for (*i = 1; *i <= text.size(); ++*i) {
            extension(i, text, suffixIndex);
        }
        *i = text.size();
        printTree(root, text, 0);

    }

};

string notes = "abcabxabcd";

int main() {
    cout << "Hello, World!" << endl;
    SuffixTree sf;
    sf.buildTree(notes);

    return 0;
}