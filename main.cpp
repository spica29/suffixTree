#include <iostream>
#include <vector>
#include <map>

using namespace std;

class SuffixTree {
    struct Node {
        //list of children
        map<char, Node> children;

        //pointer to other node via suffix link
        Node *suffixLink;
        //start and end index of edge characters
        int first;
        int *last;

        // This will be non-negative for leaves and will give index of suffix for the path from root to this leaf.
        // For non-leaf node, it will be -1.
        int suffixIndex;

        Node() : children(map<char, Node>()), suffixLink(0), first(0), last(0), suffixIndex(-1) {};
        Node(map<char, Node> _children, Node *_suffixLink, int _first, int *_last, int _suffixIndex)
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
    //returns true if first index is contained in vector of Nodes children
    /*Node* activeEdgePresentInTree(map<char, Node> *children, int _activeEdge, string text) {
        for (auto const& node : *children) {
            int firstIndex = node.second.first;
            for(int j = 1; j <= firstIndex; j++) {
                //cout << "text char: " << text[j-1] << " activeEdge char: " << text[_activeEdge - 1] << endl;
                if(text[j - 1] == text[_activeEdge - 1]){
                    //cout << "IN";
                    //cout << "j: " << j << "i: " << i;
                    //cout << children.at(key);
                    return &children->at(node.first);
                }
            }
        }
        return nullptr;
    }*/

public:
    SuffixTree() : internalNode(NULL), activeNode(root), activeEdge(-1), activeLength(0), remainingSuffixCount(0), END(new int(0)) {}
    void extension(int position, string text) {
        //extension rule 1
        this->END = &position;
        //increase remainingSuffixCount
        remainingSuffixCount++;
        //when starting new phase no internal node is waiting for it's suffix link update in current phase
        internalNode = NULL;

        while(remainingSuffixCount > 0) {
            //APCFALZ
            if(activeLength == 0){
                activeEdge = position; //position of character in string
            }

            Node *next = &activeNode->children[text[activeEdge]];
            //there does not exist edge going out from activeNode starting with activeEdge
            if(next == nullptr) {
                //extension rule 2
                //create new leaf edge
                activeNode->children[text[activeEdge]] = Node(map<char, Node>(), root, position, END, -1);
                //check suffix Link
                if(internalNode != NULL){
                    internalNode->suffixLink = activeNode;
                    internalNode == NULL;
                }
            }
            // edge exists
            else {
                //walk down
                if(walkDown(*next)){
                    continue;
                }

                //extension rule 3 - current char already on the edge
                if(text[next->first + activeLength - 1] == text[position - 1]){
                    if(internalNode != NULL && activeNode != root) {
                        internalNode->suffixLink = activeNode;
                        internalNode = NULL;
                    }

                    //APCFER3
                    activeLength++;
                    //move to next phase
                    break;
                }

                //adding new internal node and new leaf edge (extension rule 2)
                splitEnd = new int(next->first + activeLength - 1);

                //new internal node
                Node *split = new Node(map<char, Node>(), root, next->first, splitEnd, -1);
                activeNode->children[text[activeEdge]] = *split;

                //adding leaf out from internal node
                //split->children.emplace_back(Node(vector<Node>(), root, position, leafEnd, -1));
                split->children[text[position]] = Node(map<char, Node>(), root, position, END, -1);
                next->first += activeLength;
                split->children[text[next->first]] = *next;

                //add suffix link
                if(internalNode != NULL){
                    internalNode->suffixLink = split;
                }

                internalNode = split;
            }

            //suffix added to the tree
            remainingSuffixCount--;
            if (activeNode == root && activeLength > 0){ //APCFER2C1
                activeLength--;
                activeEdge = position - remainingSuffixCount + 1;
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
    void setSuffixIndexByDFS(Node *n, int labelHeight, string text)
    {
        if (n == NULL)  return;

        if (n->first != -1) //A non-root node
        {
            //Print the label on edge from parent to current node
            cout << (n->first, n->last);
        }
        int leaf = 1;
        int i;
        for (i = 0; i < text.size(); i++)
        {
            if (i < n->children.size())
            {
                if (leaf == 1 && n->first != -1)
                    printf(" [%d]\n", n->suffixIndex);

                //Current node is not a leaf as it has outgoing
                //edges from it.
                leaf = 0;
                setSuffixIndexByDFS(&n->children[i], labelHeight + edgeLength(&n->children[i]), text);
            }
        }
        if (leaf == 1)
        {
            n->suffixIndex = text.size() - labelHeight;
            printf(" [%d]\n", n->suffixIndex);
        }
    }

    void freeSuffixTreeByPostOrder(Node *n, string text)
    {
        if (n == NULL)
            return;
        int i;
        for (i = 0; i < text.size(); i++)
        {
            if (&n->children[i] != NULL)
            {
                freeSuffixTreeByPostOrder(&n->children[i], text);
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
        for (int i = 1; i <= text.size(); ++i) {
            extension(i, text);
        }
        int labelHeight = 0;
        setSuffixIndexByDFS(root, labelHeight, text);

        freeSuffixTreeByPostOrder(root, text);
    }

};

string notes = "abcabxabcd";

int main() {
    cout << "Hello, World!" << endl;
    SuffixTree sf;
    sf.buildTree(notes);

    return 0;
}