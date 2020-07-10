#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>

using namespace std;

//////////////////////////////  handleTypesAndParameters //////////////////////////////
struct Item ;
struct TreeNode ;
struct SingleItemHeader ;
struct SinglePath ;
struct SingleConditionalPatternBase ;
struct SingleFrequentPattern ;

typedef vector<int> Transaction ;
typedef vector<Transaction> Database ;

typedef vector<Item> FList ;
typedef vector<int> FrequentPattern ;

typedef vector<SinglePath> AllConditionalPath ;
typedef vector<SingleConditionalPatternBase> ConditionalPatternBases ;
typedef vector<SingleFrequentPattern> FrequentPatternTable;

struct Item{
	int itemName ;
	int itemCount ;
	Item(int itemName, int itemCount) ;
};

struct TreeNode{
	int itemName ;
	int itemCount, childrenCount ;
	weak_ptr<TreeNode> parent ;
	weak_ptr<TreeNode> nextSameItem ;
	vector<shared_ptr<TreeNode>> children;
	
	TreeNode(int itemName,shared_ptr<TreeNode> parent) ;
};

struct SingleItemHeader{	// for header_table
	Item item ;
	shared_ptr<TreeNode> header ;
	SingleItemHeader(Item item, shared_ptr<TreeNode> header) ;
};

struct SinglePath{
	vector<int> pathRecord ;
	int pathCount ;
};

struct SingleConditionalPatternBase{
	int conditionalItemName ;
	vector<SinglePath> allConditionalPaths ;
	SingleConditionalPatternBase(int conditionalItemName, const AllConditionalPath &singlePath) ;
};

struct SingleFrequentPattern{
	FrequentPattern frequentPattern ;
	int frequentPatternCount ;
	SingleFrequentPattern(FrequentPattern frequentPattern, int frequentPatternCount) ;
};

int minSupportCount = 0 ;
int transactionCount = 0 ;

//==========
Item::Item(int itemName, int itemCount){
	this->itemName = itemName ;
	this->itemCount = itemCount ;
}

//==========
TreeNode::TreeNode(int itemName, shared_ptr<TreeNode> parent):
itemName(itemName),
itemCount(1),
childrenCount(0),
parent(parent),
nextSameItem(){}

//==========
SingleItemHeader::SingleItemHeader(Item item, shared_ptr<TreeNode> header):
item(item),
header(header){}

//==========
SingleConditionalPatternBase::SingleConditionalPatternBase(int conditionalItemName, const AllConditionalPath &singlePath):
conditionalItemName(conditionalItemName),
allConditionalPaths(singlePath){}

SingleFrequentPattern::SingleFrequentPattern(FrequentPattern frequentPattern, int frequentPatternCount):
frequentPattern(frequentPattern),
frequentPatternCount(frequentPatternCount){}

////////////////////////////////////////////////////////////////////////////////////////////////

void getFrequentList(FList *fList, int minSupportCount, Database *originalDB) {	// build freq. list
	for(auto i=originalDB->begin(); i!=originalDB->end(); ++i){
		for(auto j=i->begin(); j!=i->end(); ++j){
			auto k=fList->begin() ;
			for(; k!=fList->end(); ++k) {
				if(k->itemName == *j) {	// already in freq. list 
					break ;
				}
			}
			if(k == fList->end()) {
				fList->push_back(Item(*j, 1)) ;
			}
			else {
				++(k->itemCount) ;
			}
		}
	}
	
	for(auto i=fList->begin(); i!=fList->end(); ++i) {	
		if(i->itemCount < minSupportCount){	// erase item with count < min_sup_count
			fList->erase(i) ;
			--i ;
		}
	}
	// sort freq list: descending by count , then ascending by key
	auto greaterByValue = [](Item const & a, Item const & b){
		return a.itemCount != b.itemCount?  a.itemCount > b.itemCount : a.itemName < b.itemName ;} ;
	sort(fList->begin(), fList->end(), greaterByValue) ;	
}

//==========
void updateDB(Database *originalDB, FList *fList){
	Transaction sortedTransaction ;
	for(auto i=originalDB->begin(); i!=originalDB->end(); ++i){			
		sortedTransaction.clear() ;
		for(auto j=fList->begin(); j!=fList->end(); ++j){			
			if(find(i->begin(), i->end(), j->itemName) != i->end()){			
				sortedTransaction.push_back(j->itemName) ;}}
		
		if(sortedTransaction.size() == 0) {
			originalDB->erase(i) ;
			--i ;
		}
		else {
			i->swap(sortedTransaction) ;
		}
	}
}

//==========
void printDB(Database *dataBase){
	for (auto &trans: *dataBase){
		for (auto &e: trans) {
			cout << e << ' ';
		}
		cout <<'\n';
	}
}
//==========
void printFlist(FList *fList){
	if(!fList->empty()){
		cout << "-----freq table-----\n";
		cout << "<item, freq>\n";
		for (auto &ii: *fList) {
			cout << "<" << ii.itemName << ", " << ii.itemCount << ">\n" ;
		}
	}
	else{
		cout << "fList is empty!!!!!" << '\n';
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
//  handleFPtree.hpp
class FPtree{
public:
	FPtree(FList *fList, int conditionCount) ;
	void buildFPtreeByFlistDB(Database *fListDB) ;
	void mineFPtree() ;
	void printFPtree() ;
	
private:
	shared_ptr<TreeNode> _root ;
	FList _fList ;
	vector<SingleItemHeader> _headerTable ;
	ConditionalPatternBases _conditionalPatternBases ;
	vector<int> _condition ;
	
	void insertNodeFromListAt(Transaction *itemList, shared_ptr<TreeNode> currentNode) ;
	void treeTraversal(shared_ptr<TreeNode> currentNode) ;
	
	void createConditionalPatternBases() ;
	vector<shared_ptr<FPtree>> createConditionalFPtree() ;
	void getFrequentPatterns() ;
};

//  handleFPtree.cpp
FrequentPatternTable frequentPatterns ;
//==========
FPtree::FPtree(FList *fList, int conditionCount):
_condition(),
_root(new TreeNode(-1000, nullptr)){	//root constructor:item=-1000(item:0~999), parent=NULL
	_headerTable.reserve(fList->size()) ;
	_condition.reserve(conditionCount) ;
	_fList = *fList ;
	for(auto i=_fList.begin(); i!=_fList.end(); ++i){
		shared_ptr<TreeNode> headNode(new TreeNode(i->itemName, nullptr)) ;		//use headNode to point to item's first occurence in FPtree
		_headerTable.emplace_back(SingleItemHeader(Item(i->itemName, i->itemCount), headNode)) ;
	}
}

void FPtree::buildFPtreeByFlistDB(Database *fListDB) {
	for(auto i=fListDB->begin(); i!=fListDB->end(); ++i) {
		this->insertNodeFromListAt(&(*i), _root) ;
	}
}

///////////////  Mine FPtree recursively //////////
void FPtree::mineFPtree(){
	createConditionalPatternBases() ;
	getFrequentPatterns() ;
	
	vector<shared_ptr<FPtree>> allConditionalFPtree = createConditionalFPtree() ;
	for (auto &cfpt: allConditionalFPtree) {
		//cfpt->printFPtree();
		cfpt->mineFPtree();
	}
}

//===private function
///////////////  FPtree construction //////////
void FPtree::insertNodeFromListAt(Transaction *itemList, shared_ptr<TreeNode> currentNode){
	if(!itemList->empty()) {
		int currentItem = (*itemList)[0] ;
		itemList->erase(itemList->begin()) ;	// get item and remove from itemList
		
		// use to record address from header to the node before new node
		shared_ptr<TreeNode> *pointerToPreviousNode = nullptr ;	
		for (auto i=_headerTable.begin(); i!=_headerTable.end(); ++i) {
			if (currentItem == i->item.itemName) {			// find item in _headerTable
				shared_ptr<TreeNode> previousNode = i->header ;
				while (!previousNode->nextSameItem.expired()) {		// go further until node has no nextSameItem
					previousNode = previousNode->nextSameItem.lock() ;
				}
				pointerToPreviousNode = &(previousNode) ;	// get address
				break ;
			}
		}
		
		bool DuplicatePath = false ;
		if (currentNode->childrenCount != 0) {
			// iterate each child
			for (auto i=currentNode->children.begin(); i!=currentNode->children.end(); ++i) {
				if (currentItem == (*i)->itemName) {	// item is in child: path exist
					++(*i)->itemCount ;
					insertNodeFromListAt(itemList, *i) ; // insert to next level
					DuplicatePath = true ;
					break ;
				}
			}
		}
		
		if (!DuplicatePath) {
			currentNode->children.emplace_back(new TreeNode(currentItem, currentNode)) ;
			++currentNode->childrenCount ;
			// connect with previous same items
			(*pointerToPreviousNode)->nextSameItem = currentNode->children.back() ;	// .back(): Returns a reference to the last element in the vector.
			insertNodeFromListAt(itemList, currentNode->children.back()) ;
		}
	}
}

void FPtree::createConditionalPatternBases(){
	weak_ptr<TreeNode> sameItem, currentItem ;
	SinglePath singlePath ;
	AllConditionalPath allPaths ;
	
	singlePath.pathRecord.reserve(512) ;
	allPaths.reserve(512) ;
	_conditionalPatternBases.reserve(_headerTable.size()) ;
	
	for(auto i=_headerTable.end()-1; i!=_headerTable.begin()-1; --i){
		sameItem = i->header->nextSameItem ;	//point to item's first occurence in FPtree (stored in header)
		
		while(!sameItem.expired()){		//iterate all same items in FPtree to get ConditionalPatternBase
			auto tmpSameItem = sameItem.lock() ;

			// Except suffix (the item we are examining), set each node’s value as the sum of its child’s values (from leaves to root)
			currentItem = tmpSameItem->parent ;
			singlePath.pathCount = tmpSameItem->itemCount ;
			
			while (!currentItem.expired()) {	// iterate all parents and record path
				auto tmpCurrentItem = currentItem.lock() ;
				singlePath.pathRecord.push_back(tmpCurrentItem->itemName) ;
				currentItem = tmpCurrentItem->parent ;
			}
			
			if(singlePath.pathRecord.size() > 0){
				singlePath.pathRecord.pop_back() ;	// pop root
				reverse(singlePath.pathRecord.begin(), singlePath.pathRecord.end()) ;	//change to root->leaf
				allPaths.push_back(singlePath) ;	//record all item's path and count
				singlePath.pathRecord.clear() ;
			}
			sameItem = tmpSameItem->nextSameItem ;	// point to next same item in FPtree
		}
		
		if(allPaths.size() > 0){	// record this item's ConditionalPatternBase
			_conditionalPatternBases.emplace_back(SingleConditionalPatternBase(i->item.itemName, allPaths)) ;
			allPaths.clear() ;
		}
	}
}

void FPtree::getFrequentPatterns(){
	/*
		ConditionalPatternBases = vector<SingleConditionalPatternBase> 
		= vector< int item_name, vecotor<SinglePath> allConditionalPaths>
	*/
	for (auto i=_conditionalPatternBases.begin(); i!=_conditionalPatternBases.end(); ++i) {
		int pathCount = 0 ;
		for (auto j=i->allConditionalPaths.begin(); j!=i->allConditionalPaths.end(); ++j) {	//紀錄各condition下 所有path出現次數總和
			pathCount += j->pathCount ;
		}

		FrequentPattern currentCondition = _condition ; // vector<int>
		// add item name to its condition (as suffix)
		currentCondition.push_back(i->conditionalItemName) ;
		sort(currentCondition.begin(), currentCondition.end()) ;
		frequentPatterns.emplace_back(SingleFrequentPattern(currentCondition, pathCount)) ;
	}
}

vector<shared_ptr<FPtree>> FPtree::createConditionalFPtree(){
	vector<shared_ptr<FPtree>> allConditionalFPtrees;
	FList conditionalFList ;
	Database conditionalFListDB ;
	
	conditionalFList.reserve(_fList.size()) ;	// conditionalFList always <= _fList
	conditionalFListDB.reserve(_fList.size()) ;
	
	for (auto i=_conditionalPatternBases.begin(); i!=_conditionalPatternBases.end(); ++i) {	// iterate all paths of the item
		conditionalFList = _fList ;
		for (auto j=conditionalFList.begin(); j!=conditionalFList.end();++j) {	// init conditionalFList
			j->itemCount = 0 ;
		}
		for (auto j=i->allConditionalPaths.begin(); j!=i->allConditionalPaths.end(); ++j) {	// iterate all path
			for (auto k=j->pathRecord.begin(); k!=j->pathRecord.end(); ++k) {	// iterate all items in path
				for (auto l=conditionalFList.begin(); l!=conditionalFList.end(); ++l) {	
					if (l->itemName == *k) {	// 
						l->itemCount += j->pathCount ;
						break ;
					}
				}
			}
			conditionalFListDB.insert(conditionalFListDB.end(), j->pathCount, j->pathRecord) ;	// record this path
		}
		
		for (auto j=conditionalFList.end()-1; j!=conditionalFList.begin()-1; --j) {
			if (j->itemCount < minSupportCount) {	// erase from conditionalFList
				for (auto k=conditionalFListDB.end()-1; k!=conditionalFListDB.begin()-1; --k) {
					for (auto l=k->end()-1; l!=k->begin()-1; --l) {
						if(*l == j->itemName) {		// erase from conditionalFListDB
							k->erase(l) ;
						}
					}
				}
				conditionalFList.erase(j) ;
			}}
		
		if(conditionalFList.size() > 0){
			shared_ptr<FPtree> newConditionalFPtree = shared_ptr<FPtree>(new FPtree(&conditionalFList, (int)(this->_condition.size()+1))) ;
			newConditionalFPtree->buildFPtreeByFlistDB(&conditionalFListDB) ;
			newConditionalFPtree->_condition = this->_condition ;
			newConditionalFPtree->_condition.push_back(i->conditionalItemName) ;
			allConditionalFPtrees.push_back(newConditionalFPtree) ;
		}
		conditionalFList.clear() ;
		conditionalFListDB.clear() ;
	}
	
	return allConditionalFPtrees ;
}

////////////// FreqPattern///////////////////////////////////////////////////////////////////

static void sortFrequentPatternsByLength() ;
static void printFrequentPatternTable(int transactionCount) ;
static void writeFrequentPatternTableToFile(string fileName, int transactionCount) ;

//==========
static void sortFrequentPatternsByLength(){
	auto greaterBySizeThenValue = [](SingleFrequentPattern const &a, SingleFrequentPattern const &b){
		if(a.frequentPattern.size() != b.frequentPattern.size()){
			return a.frequentPattern.size() < b.frequentPattern.size() ;}
		else{
			for(int i=0; i<a.frequentPattern.size(); ++i){
				if(a.frequentPattern[i] < b.frequentPattern[i]){
					return true ;}
				else if(a.frequentPattern[i] > b.frequentPattern[i]){
					return false ;}
			}
			return false ;
		}} ;			
		
	sort(frequentPatterns.begin(), frequentPatterns.end(), greaterBySizeThenValue) ;
}

// static void printFrequentPatternTable(int transactionCount){
// 	for(auto i=frequentPatterns.begin(); i!=frequentPatterns.end(); ++i){
// 		auto j=i->frequentPattern.begin() ;
// 		for(; j!=i->frequentPattern.end()-1; ++j) {
// 			cout << *j << "," ;
// 		}
// 		cout << *j << ":" << fixed << setprecision(4) << (double)i->frequentPatternCount/transactionCount << "\n" ;
// 	}
// }

static void writeFrequentPatternTableToFile(string fileName, int transactionCount){
	ofstream fout(fileName) ;
	for(auto i=frequentPatterns.begin(); i!=frequentPatterns.end(); ++i){
		auto j=i->frequentPattern.begin() ;
		for(; j!=i->frequentPattern.end()-1; ++j){
			fout << *j << "," ;}
		fout << *j << ":" << fixed << setprecision(4) << (double)i->frequentPatternCount/transactionCount << "\n" ;
	}
	fout.close() ;
}



int main(int argc, const char * argv[]) {
	double minSupportRatio = atof(argv[1]);
	string readFileName = (string)argv[2];
	string writeFileName = (string)argv[3];
	
	Database originalDB; // vector<Transaction>

	// scan file
	ifstream in_file(readFileName);
	if (in_file.is_open() && in_file.good()) {
		string line;
		Transaction singleTransaction;
		singleTransaction.reserve(100);
		originalDB.reserve(128);
	
		while (getline(in_file, line)) {
			singleTransaction.clear();
			istringstream ssline(line);
			string singleItem;	
			while(getline(ssline, singleItem, ',')) {
			singleTransaction.push_back(stoi(singleItem)) ;
			}
			originalDB.push_back(singleTransaction) ;
		}
		in_file.close() ;
	} else {
		cout << "Failed to open in_file...";
	}
	//printDB(&originalDB) ;

	transactionCount = (int)originalDB.size();			
	minSupportCount = transactionCount * minSupportRatio;
	cout << "min_count: " << minSupportCount << '\n';

	FList fList ;
	getFrequentList(&fList, minSupportCount, &originalDB);	
	printFlist(&fList) ;

	// update DB according to freq. list
	updateDB(&originalDB, &fList);
	Database &fListDB = originalDB;
	// cout << "----------New DB----------\n";
	// printDB(&fListDB);
	// cout << "---------------------------\n";

	FPtree fpTree(&fList, 0);
	// build FPtree using fListDB
	fpTree.buildFPtreeByFlistDB(&fListDB);
	//fpTree.printFPtree() ;
	fpTree.mineFPtree();

	sortFrequentPatternsByLength() ;
	writeFrequentPatternTableToFile(writeFileName, transactionCount);	// write freq. pattern + support
	cout << "Total FP count: " << frequentPatterns.size() << "\n";
	cout << "Finsh writing file" << "\n";
}