#include <iostream>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <limits>
#include <fstream>
#include <cstdlib>
#include <algorithm>

using namespace std;

// ---------------- STRUCTS ----------------
struct Case {
    int caseId;
    string description;
    int priority;
};

struct ProgressNode {
    string stage;
    ProgressNode* next;
    ProgressNode(string s) : stage(s), next(nullptr) {}
};

struct CaseProgress {
    ProgressNode* head = nullptr;
    ProgressNode* tail = nullptr;
};

struct AVLNode {
    int caseId;
    int height;
    AVLNode* left;
    AVLNode* right;
    AVLNode(int id) : caseId(id), height(1), left(nullptr), right(nullptr) {}
};

// ---------------- AVL FUNCTIONS ----------------
int height(AVLNode* n) { return n ? n->height : 0; }
int balance(AVLNode* n) { return n ? height(n->left) - height(n->right) : 0; }
void updateHeight(AVLNode* n) { if (n) n->height = 1 + max(height(n->left), height(n->right)); }

AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* t = x->right;
    x->right = y;
    y->left = t;
    updateHeight(y);
    updateHeight(x);
    return x;
}

AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* t = y->left;
    y->left = x;
    x->right = t;
    updateHeight(x);
    updateHeight(y);
    return y;
}

AVLNode* insertAVL(AVLNode* node, int id) {
    if (!node) return new AVLNode(id);
    if (id < node->caseId) node->left = insertAVL(node->left, id);
    else if (id > node->caseId) node->right = insertAVL(node->right, id);
    else return node;

    updateHeight(node);
    int bf = balance(node);
    if (bf > 1 && id < node->left->caseId) return rotateRight(node);
    if (bf < -1 && id > node->right->caseId) return rotateLeft(node);
    if (bf > 1 && id > node->left->caseId) { node->left = rotateLeft(node->left); return rotateRight(node); }
    if (bf < -1 && id < node->right->caseId) { node->right = rotateRight(node->right); return rotateLeft(node); }
    return node;
}

AVLNode* findMin(AVLNode* n) {
    while (n && n->left) n = n->left;
    return n;
}

AVLNode* deleteAVL(AVLNode* root, int id) {
    if (!root) return nullptr;
    if (id < root->caseId) root->left = deleteAVL(root->left, id);
    else if (id > root->caseId) root->right = deleteAVL(root->right, id);
    else {
        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            delete root;
            return temp;
        } else {
            AVLNode* temp = findMin(root->right);
            root->caseId = temp->caseId;
            root->right = deleteAVL(root->right, temp->caseId);
        }
    }
    updateHeight(root);
    int bf = balance(root);
    if (bf > 1 && balance(root->left) >= 0) return rotateRight(root);
    if (bf > 1 && balance(root->left) < 0) { root->left = rotateLeft(root->left); return rotateRight(root); }
    if (bf < -1 && balance(root->right) <= 0) return rotateLeft(root);
    if (bf < -1 && balance(root->right) > 0) { root->right = rotateRight(root->right); return rotateLeft(root); }
    return root;
}

void inorder(AVLNode* n) {
    if (!n) return;
    inorder(n->left);
    cout << n->caseId << " ";
    inorder(n->right);
}

// ---------------- CASE MANAGER ----------------
unordered_map<int, Case> caseMap;
unordered_map<int, CaseProgress> progressMap;
AVLNode* avlRoot = nullptr;
priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
stack<string> undoStack;

// --------------- UTILITIES ----------------
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int readInt(const string& msg) {
    int val;
    while (true) {
        cout << msg;
        if (cin >> val) {
            // consume leftover newline so getline works afterwards
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return val;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Enter number again.\n";
    }
}

string readLine(const string& msg) {
    cout << msg;
    string s;
    getline(cin, s);
    return s;
}

void pauseAndClear() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    clearScreen();
}

// ---------------- CORE FUNCTIONS ----------------
bool exists(int id) { return caseMap.find(id) != caseMap.end(); }

void rebuildPQ() {
    while (!pq.empty()) pq.pop();
    for (auto &kv : caseMap) pq.push({kv.second.priority, kv.first});
}

bool addCase() {
    int id = readInt("Enter Case ID: ");
    if (exists(id)) { cout << "Case already exists!\n"; return false; }
    string desc = readLine("Enter Description: ");
    int pr = readInt("Enter Priority (lower = higher priority): ");
    caseMap[id] = {id, desc, pr};
    progressMap[id] = CaseProgress();
    avlRoot = insertAVL(avlRoot, id);
    pq.push({pr, id});
    undoStack.push("ADD " + to_string(id));
    cout << "\nCase added successfully.\n";
    return true;
}

bool deleteCase() {
    int id = readInt("Enter Case ID to delete: ");
    if (!exists(id)) { cout << "Case not found!\n"; return false; }
    undoStack.push("DEL " + to_string(id));
    caseMap.erase(id);
    progressMap.erase(id);
    avlRoot = deleteAVL(avlRoot, id);
    rebuildPQ();
    cout << "\nCase deleted.\n";
    return true;
}

bool updateCase() {
    int id = readInt("Enter Case ID to update: ");
    if (!exists(id)) { cout << "Case not found!\n"; return false; }
    string newDesc = readLine("Enter new Description: ");
    int newPr = readInt("Enter new Priority: ");
    caseMap[id].description = newDesc;
    caseMap[id].priority = newPr;
    rebuildPQ();
    cout << "\nCase updated successfully.\n";
    return true;
}

void showAll() {
    if (caseMap.empty()) { cout << "No cases available.\n"; return; }
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> temp = pq;
    cout << "\nPriority Order (lower = higher priority):\n";
    cout << "-----------------------------------------\n";
    while (!temp.empty()) {
        int id = temp.top().second;
        temp.pop();
        cout << "Case ID: " << id << " | Priority: " << caseMap[id].priority
             << " | Description: " << caseMap[id].description << "\n";
    }
}

void addProgress() {
    int id = readInt("Enter Case ID to add progress to: ");
    if (!exists(id)) { cout << "Case not found!\n"; return; }

    cout << "\nAvailable stages:\n";
    cout << "1. Case Registered\n2. Under Investigation\n3. Hearing Scheduled\n4. Judgment Passed\n5. Case Closed\n";
    int opt = readInt("Enter stage number (1-5): ");
    string stage;
    switch (opt) {
        case 1: stage = "Case Registered"; break;
        case 2: stage = "Under Investigation"; break;
        case 3: stage = "Hearing Scheduled"; break;
        case 4: stage = "Judgment Passed"; break;
        case 5: stage = "Case Closed"; break;
        default: stage = readLine("Enter custom stage: "); break;
    }

    ProgressNode* node = new ProgressNode(stage);
    CaseProgress &cp = progressMap[id];
    if (!cp.head) cp.head = cp.tail = node;
    else { cp.tail->next = node; cp.tail = node; }

    cout << "\nStage added successfully.\n";
    cout << "Progress for Case ID " << id << ":\n";
    ProgressNode* temp = cp.head;
    while (temp) { cout << "- " << temp->stage << "\n"; temp = temp->next; }
}

void showProgress() {
    int id = readInt("Enter Case ID to show progress: ");
    if (!exists(id)) { cout << "Case not found!\n"; return; }
    ProgressNode* temp = progressMap[id].head;
    if (!temp) { cout << "No progress recorded.\n"; return; }
    cout << "\nProgress for Case ID " << id << ":\n";
    while (temp) { cout << "- " << temp->stage << "\n"; temp = temp->next; }
}

void saveToFile() {
    ofstream file("case_records.txt");
    for (auto &kv : caseMap) {
        file << "Case ID: " << kv.second.caseId << "\n";
        file << "Description: " << kv.second.description << "\n";
        file << "Priority: " << kv.second.priority << "\nProgress:\n";
        ProgressNode* p = progressMap[kv.first].head;
        if (!p) file << "(no progress)\n";
        while (p) { file << "- " << p->stage << "\n"; p = p->next; }
        file << "\n";
    }
    file.close();
    cout << "\nData saved to case_records.txt successfully.\n";
}

// ---------------- MAIN MENU ----------------
int main() {
    int choice;
    while (true) {
        clearScreen();
        cout << "=========================================\n";
        cout << "       CASE MANAGEMENT SYSTEM\n";
        cout << "=========================================\n";
        cout << "1. Add Case\n2. Delete Case\n3. Update Case\n4. View All Cases\n";
        cout << "5. Add Progress\n6. Show Progress\n7. Show AVL Tree (in-order)\n8. Save All Data to File\n0. Exit\n";
        cout << "-----------------------------------------\n";

        choice = readInt("Enter choice: ");
        switch (choice) {
            case 1: addCase(); break;
            case 2: deleteCase(); break;
            case 3: updateCase(); break;
            case 4: showAll(); break;
            case 5: addProgress(); break;
            case 6: showProgress(); break;
            case 7: cout << "\nAVL Tree (in-order): "; inorder(avlRoot); cout << "\n"; break;
            case 8: saveToFile(); break;
            case 0: cout << "\nExiting... Goodbye!\n"; return 0;
            default: cout << "\nInvalid choice!\n";
        }

        cout << endl;
        pauseAndClear();
    }
}

