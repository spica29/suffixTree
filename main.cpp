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
                activeNode->children[text[activeEdge - 1]]->parent = activeNode;
                activeNode->children[text[activeEdge - 1]]->suffixIndices.emplace_back(*suffixIndex);
                //update suffixIndices for parents of new node
                Node *newLeaf = activeNode->children[text[activeEdge - 1]];
                while(newLeaf->parent != NULL){
                    newLeaf->parent->suffixIndices.emplace_back(*suffixIndex);
                    newLeaf = newLeaf->parent;
                }
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
                split->parent = next->parent;
                split->suffixIndices.insert(split->suffixIndices.begin(), next->suffixIndices.begin(), next->suffixIndices.end());
                activeNode->children[text[activeEdge - 1]] = split;

                //adding leaf out from internal node
                split->children[text[*position - 1]] = new Node(map<char, Node*>(), root, *position, END, *suffixIndex);
                split->children[text[*position - 1]]->parent = split;
                split->children[text[*position - 1]]->suffixIndices.emplace_back(*suffixIndex);
                (*suffixIndex)++;
                next->first += activeLength;
                next->parent = split;
                split->children[text[*split->last]] = next;
                //update suffixIndices for parents of new node
                Node *newLeaf = split->children[text[*position - 1]];
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

    //void printGraphviz(Node *n, string text, char *letter_parent, char *letter_child, bool root_visited){
    void printGraphviz(Node *n, string text, int *letter_parent, int *letter_child, bool root_visited){
        if(n == NULL) {
            *letter_child = *letter_child + 1;
            return;
        }

        for (auto const& node : n->children) {
            if(n == root && root_visited){
                *letter_parent = 0;
            }
            //cout << "first: " << n->first << ", last: " <<  *n->last << ", suffix index: " << n->suffixIndex << endl;
            //cout << letter_parent << " -> " << letter_child << endl;
            /*
            if(n->first == 0){
                cout << (string)"\"" + letter_parent + (string)" root \"" << " -> " << (string)"\"" + letter_child + (string)" " + text.substr(node.second->first - 1, *node.second->last - node.second->first + 1 ) + (string)"\"" << endl;
            }
            else cout << (string)"\"" + letter_parent + (string)" " + text.substr(n->first - 1, *n->last - n->first + 1) + (string)"\"" << " -> " << (string)"\"" + letter_child + (string)" " + text.substr(node.second->first - 1, *node.second->last - node.second->first + 1) + (string)"\"" << endl;
             */
            std::fstream file;
            file.open (".././output.txt", std::fstream::in | std::fstream::out | std::fstream::app);

            if (!file) { std::cerr<<"Error writing to ..."<<std::endl; }
            else
                file  << *letter_parent << " -> " << *letter_child << " [ label=\"" + text.substr(node.second->first - 1, *node.second->last - node.second->first + 1) + "\" ];" << endl;
            file.close();
            if(root_visited && *letter_parent == 0){
                *letter_parent = *letter_child;
            }
            else {
                (*letter_parent)++;
            }
            if(n == root){
                root_visited = true;
            }
            (*letter_child)++;
            printGraphviz(node.second, text, letter_parent, letter_child, root_visited);
        }
        (*letter_parent)--;
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
        root->last = rootEnd;

        activeNode = root;
        int *suffixIndex = new int(1);
        int *i = new int(1);
        for (*i = 1; *i <= text.size(); ++*i) {
            extension(i, text, suffixIndex);
        }
        *i = text.size();
        //printTreeDFS(root, text);
        char *first = new char('a');
        char *child = new char('b');
        int *first_num = new int(0);
        int *child_num = new int(1);
        //printGraphviz(root, text, first, child, false);
        printGraphviz(root, text, first_num, child_num, false);
    }

};

string notes = "abcabxabcd";

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