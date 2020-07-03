#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

class Transaction {
public:
    void add_item(int item) {
        items.push_back(item);
    }

    vector<int> get_tx() {
        return items;
    }

    void print_tx() {
        for (auto &v:items) {
            cout << v << ' ';
        }
    }
    
private:
    vector<int> items;
};

class Tree {
private:
    struct Node { 
        int item; 
        int sup; 
        vector<Node*> child; 
        Node *parent;
    };
    Node *root;
    void projTableRecursion(int val, int *count, vector<Transaction> *projTable, map<int,int> *projHeaderTable, Node *node);
public: 
    Tree() {
        root = new Node;
        root->item = 0;
        root->sup = 0;
        root->parent = NULL;
    }
    void insert(vector<int> trans); 
    void projTable(int val, int count, vector<Transaction> *projTable, map<int,int> *projHeaderTable); 
    void printTree();
};

void Tree::insert(vector<int> trans) {
    Node *current_node = root;
    bool added = false;
    bool createNewBranch = false;

    if (root->child.empty()) {
        // add each element of the transaction
        for (auto &ele:trans) {
            Node *new_node = new Node;
            new_node->item = ele;
            new_node->sup = 1;
            new_node->parent = current_node;

            current_node->child.push_back(new_node);
            current_node = current_node->child.at(0);
        }
    } else {
        for (auto &ele:trans) {
            if (!createNewBranch) {
                // iterate through vector<*Node> child
                for (auto &c:current_node->child) {
                    // find if current item is in child
                    if (ele == c->item) {
                        c->sup++;
                        current_node = c;
                        added = true;
                        break;
                    }
                }
                if (!added) {
                    Node *new_node = new Node;
                    new_node->item = ele;
                    new_node->sup = 1;
                    new_node->parent = current_node;
                    current_node->child.push_back(new_node);
                    current_node = new_node;
                    createNewBranch = true;
                } else {
                    added = false;
                }

            } else {
                Node *new_node = new Node;
                new_node->item = ele;
                new_node->sup = 1;
                new_node->parent = current_node;
                current_node->child.push_back(new_node);
                current_node = new_node;
            }

        }
    }
}


void Tree::projTable(
    int val, int count, vector<Transaction> *projTable,
    map<int, int> *projHeaderTable
) {
    for (auto &c:root->child) {
        projTableRecursion(val, &count, projTable, projHeaderTable, c);
    }
    for (size_t i = 0; i < root->child.size(); i++) {
        projTableRecursion(val, &count, projTable, projHeaderTable, root->child.at(i));
    }
}


void Tree::projTableRecursion(int val, int *count, vector<Transaction> *projTable, map<int, int> *projHeaderTable, Node *node) {
    Transaction itemSet;
    Node *currentNode = node;

    if (node->item == val) {
        currentNode = currentNode->parent;
        while (currentNode != root) {
            itemSet.add_item(currentNode->item);
            (*projHeaderTable)[currentNode->item] += node->sup;
            currentNode = currentNode->parent;
        }
        for (size_t i = 0; i < node->sup; i++) {
            (*projTable).push_back(itemSet);
            (*count)--;
        }
    } else {
        if (*count > 0) {
            for (auto &c:node->child) {
                projTableRecursion(val, count, projTable, projHeaderTable, c);
            }
        }
    }
}

// global data structures
vector<Transaction> DB;
//vector<HeaderNode> header_table;
map<int, int> freq_table;
vector<pair<int,int>> header_table;

Tree fpTree;
vector<pair<vector<int>,int> > frequentPatterns;

// for sorting freq_table
bool compare_freq_table(pair<int,int> i, pair<int,int> j) {
    return (i.second > j.second);
}

void get_header_table(int min_count) {
    for (auto &ii: freq_table) {
        if (ii.second >= min_count) {
            header_table.push_back(ii);
        }
    }
    sort(header_table.begin(), header_table.end(), compare_freq_table);
}

void update_freq_table(int min_count) {
    // map<int, int> map_temp;
    // map_temp.clear();

    // for (auto &ii:freq_table) {
    //     if (ii.second >= min_count) {
    //         map_temp[ii.first] = ii.second;
    //     }
    // }

    // freq_table.clear();
    // freq_table = map_temp;
    // map_temp.clear();
}


void updateDB() {
    vector<Transaction> tempDB;
    tempDB.clear();

    for (auto &tx: DB) {
        Transaction tempTx;
        for (auto &row: header_table) {
            auto trans = tx.get_tx();
            if (find(trans.begin(), trans.end(), row.first) !=  trans.end()) {
                tempTx.add_item(row.first);
            }
        }
        if (!tempTx.get_tx().empty())
            tempDB.push_back(tempTx);
    }

    DB.clear();
    DB = tempDB;
}


void projectTables(
    int min_count,
    vector<pair<int,int>> header_table,
    Tree fpTree,
    vector<pair<vector<int>,int>> *frequentPatterns,
    vector<int> currentPattern
){
    pair<vector<int>,int> patternWithCount;
    
    if (header_table.size() > 1) {
        for (size_t i = header_table.size() - 1; i > 0; i--) {
            vector<Transaction> *projTransactions = new vector<Transaction>;
            map<int,int> *projHeaderTable = new map<int,int>;

            vector<pair<int,int> > sortedProjHeaderTable;
            Tree projTree;

            // Add current frequent pattern to the list
            currentPattern.push_back(header_table.at(i).first);

            patternWithCount.first = currentPattern;
            patternWithCount.second = header_table.at(i).second;

            (*frequentPatterns).push_back(patternWithCount);

            // Create the projected transactions and header table for the current item
            fpTree.projTable(header_table.at(i).first, header_table.at(i).second, projTransactions, projHeaderTable);

            // Sort the projected header table and transactions
            //get_header_table(min_count, projHeaderTable, &sortedProjHeaderTable);
            //updateDB(&sortedProjHeaderTable, projTransactions);

            // Create the projected tree
            for (auto &pt: *projTransactions) {
                projTree.insert(pt.get_tx());
            }

            // recursive cal
            if (!sortedProjHeaderTable.empty()) {
                projectTables(
                    min_count,
                    sortedProjHeaderTable,
                    projTree,
                    frequentPatterns,
					currentPattern
                );
                currentPattern.pop_back();
            } else {
                cout << "sortedProjHeaderTable.empty！！\n";
            }
            // Add the current frequent pattern to the list
            currentPattern.push_back(header_table.at(0).first);
            patternWithCount.first = currentPattern;
		    patternWithCount.second = header_table.at(0).second;
		    (*frequentPatterns).push_back(patternWithCount);
        }
    } else if (header_table.size() == 1) {
        // Add the current frequent pattern to the list
        currentPattern.push_back(header_table.at(0).first);
        patternWithCount.first = currentPattern;
		patternWithCount.second = header_table.at(0).second;
		(*frequentPatterns).push_back(patternWithCount);
    }

}
	

void printPatterns(vector<pair<vector<int>,int> > frequentPatterns)
{
	for (size_t i = 0; i < frequentPatterns.size(); i++)
	{
		for (size_t j = 0; j < frequentPatterns.at(i).first.size(); j++)
		{
			cout << frequentPatterns.at(i).first.at(j) << " ";
		}
		cout << "(" << frequentPatterns.at(i).second << ")" << endl;
	}
}


int main(int argc, char *argv[]) {
    double min_support = stod(argv[1]);
    ifstream in_file(argv[2]);
    //ofstream out_file(argv[3]);
    int tx_count = 0;

    // scan dataset
    if (in_file.is_open() && in_file.good()) {
        string line = "";
        while (getline(in_file, line)) {
            //cout << line << '\n';
            istringstream iss(line);
            Transaction tx;
            string str;
            while (getline(iss, str, ',')) {
                //cout << str << ' ';
                int item= stoi(str);
                tx.add_item(item);
                if (freq_table.find(item) != freq_table.end()) {
                    freq_table[item] += 1;
                } else {
                    freq_table[item] = 1;
                }
            }
            //cout << '\n';
            tx_count += 1;
            DB.push_back(tx);
        }
        in_file.close();
    } else {
        cout << "Failed to open in_file...";
    }

    int min_count = min_support*tx_count;
    cout << "tx_count: " << tx_count <<'\n';
    cout << "min_count: " << min_count <<'\n';
    cout << "----init freq table-----\n";
    for (auto &ii: freq_table) {
        cout << ii.first << ": " << ii.second << '\n';
    }
    //update_freq_table(min_count);
    get_header_table(min_count);
    cout << "-----new freq table-----\n";
    for (auto &ii: header_table) {
        cout << ii.first << ": " << ii.second << '\n';
    }
    // exclude non-frequent items in DB, sort items by freq.
    updateDB();
    cout << "-------new DB-------\n";
	for (auto &t: DB) {
        t.print_tx();
        cout << '\n';
    }

    // create tree
    // for (auto &tx:DB) {
    //     vector<int> trans = tx.get_tx();
    //     fpTree.insert(trans);
    // }

    // create projection table, their header tables, and their trees
    // vector<int> currentPattern;
    // projectTables(
	// 	min_count,
	// 	header_table,
	// 	fpTree,
	// 	&frequentPatterns,
	// 	currentPattern
	// );

    // printPatterns(frequentPatterns);
	// cout << endl;

    return 0;
}