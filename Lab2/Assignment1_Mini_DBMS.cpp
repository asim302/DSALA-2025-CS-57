// Mini Database Engine - DSA Lab Work Assignment
// A simplified SQL-like in-memory database engine in C++

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>

using namespace std;

//CONSTRAINT FLAGS (Bitwise)
const unsigned int CONSTRAINT_PRIMARY_KEY = 1;   // 001
const unsigned int CONSTRAINT_NOT_NULL = 2;      // 010
const unsigned int CONSTRAINT_UNIQUE = 4;        // 100


class Column {
public:
    string name;
    string type;  // "int" or "string"
    unsigned int constraints;

// Constructors
    Column() : constraints(0) {}

    Column(string colName, string colType, unsigned int colConstraints)
        : name(colName), type(colType), constraints(colConstraints) {}

    //Methods of a Class: Check if constraint is set using bitwise AND
    bool isPrimaryKey() 
    const 
    {
        return (constraints & CONSTRAINT_PRIMARY_KEY) != 0;
    }

    bool isNotNull() 
    const
     {
        return (constraints & CONSTRAINT_NOT_NULL) != 0;
    }

    bool isUnique()
     const
    {
        return (constraints & CONSTRAINT_UNIQUE) != 0;
    }
};

class Row {
public:
    vector<string> values;

    Row() {}

    Row(vector<string> rowValues) : values(rowValues) {}

    string getValue(int index) const {
        if (index >= 0 && index < (int)values.size()) {
            return values[index];
        }
        return "";
    }

    void setValue(int index, const string& value) {
        if (index >= 0 && index < (int)values.size()) {
            values[index] = value;
        }
    }
};

class Table {
public:
    string tableName;
    vector<Column> columns;
    vector<Row*> rows;  
    Table() {}

    Table(string name) : tableName(name) {}

    ~Table() {
        clearRows();
    }

    void clearRows() {
        for (Row* row : rows) {
            delete row;
            row = nullptr;
        }
        rows.clear();
    }

    void addColumn(const Column& col) {
        columns.push_back(col);
    }

    int getColumnIndex(const string& colName) const {
        for (int i = 0; i < (int)columns.size(); i++) {
            if (columns[i].name == colName) {
                return i;
            }
        }
        return -1;
    }

    bool validateRow(const Row& row) const {
        for (int i = 0; i < (int)columns.size(); i++) {
            if (columns[i].isNotNull()) {
                string value = row.getValue(i);
                string trimmed = value;
                trimmed.erase(remove_if(trimmed.begin(), trimmed.end(), ::isspace), trimmed.end());
                if (trimmed.empty()) {
                    cout << "Error: Column '" << columns[i].name << "' cannot be NULL." << endl;
                    return false;
                }
            }
        }

        for (int i = 0; i < (int)columns.size(); i++) {
            if (columns[i].isPrimaryKey()) {
                string pkValue = row.getValue(i);
                for (Row* existingRow : rows) {
                    if (existingRow->getValue(i) == pkValue) {
                        cout << "Error: Duplicate primary key value '" << pkValue << "'." << endl;
                        return false;
                    }
                }
            }
        }

        for (int i = 0; i < (int)columns.size(); i++) {
            if (columns[i].isUnique()) {
                string value = row.getValue(i);
                for (Row* existingRow : rows) {
                    if (existingRow->getValue(i) == value) {
                        cout << "Error: Duplicate unique value '" << value << "' in column '" 
                             << columns[i].name << "'." << endl;
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool insertRow(const vector<string>& values) {
        if ((int)values.size() != (int)columns.size()) {
            cout << "Error: Column count mismatch. Expected " << columns.size() 
                 << " but got " << values.size() << "." << endl;
            return false;
        }

        Row* newRow = new Row(values);  // Dynamic allocation

        if (!validateRow(*newRow)) {
            delete newRow;
            return false;
        }

        rows.push_back(newRow);
        return true;
    }

    void displayAll() const {
        if (rows.empty()) {
            cout << "No records found." << endl;
            return;
        }

        for (int i = 0; i < (int)columns.size(); i++) {
            cout << columns[i].name << "\t";
        }
        cout << endl;
        for (int i = 0; i < (int)columns.size(); i++) {
            cout << "----";
        }
        cout << endl;

        for (Row* row : rows) {
            for (int i = 0; i < (int)row->values.size(); i++) {
                cout << row->values[i] << "\t";
            }
            cout << endl;
        }
    }

    vector<Row*> selectWhere(const string& colName, const string& value) const {
        vector<Row*> result;
        int colIndex = getColumnIndex(colName);
        
        if (colIndex == -1) {
            cout << "Error: Column '" << colName << "' not found." << endl;
            return result;
        }

        for (Row* row : rows) {
            if (row->getValue(colIndex) == value) {
                result.push_back(row);
            }
        }

        return result;
    }

    int deleteWhere(const string& colName, const string& value) {
        int colIndex = getColumnIndex(colName);
        
        if (colIndex == -1) {
            cout << "Error: Column '" << colName << "' not found." << endl;
            return 0;
        }

        int count = 0;
        vector<Row*> rowsToDelete;

        for (Row* row : rows) {
            if (row->getValue(colIndex) == value) {
                rowsToDelete.push_back(row);
                count++;
            }
        }

        for (Row* row : rowsToDelete) {
            auto it = find(rows.begin(), rows.end(), row);
            if (it != rows.end()) {
                rows.erase(it);
                delete row;
            }
        }

        return count;
    }

    void saveToFile(ofstream& outFile) const {
        outFile << "TABLE " << tableName << endl;
        
        for (const Column& col : columns) {
            outFile << col.name << " " << col.type << " " << col.constraints << endl;
        }
        
        outFile << "DATA" << endl;
        
        for (Row* row : rows) {
            for (int i = 0; i < (int)row->values.size(); i++) {
                outFile << row->values[i];
                if (i < (int)row->values.size() - 1) {
                    outFile << " ";
                }
            }
            outFile << endl;
        }
    }

    bool loadFromFile(ifstream& inFile) {
        string line;
        
        if (!getline(inFile, line)) return false;
        if (line.substr(0, 6) != "TABLE ") {
            cout << "Error: Invalid file format." << endl;
            return false;
        }
        tableName = line.substr(6);

        columns.clear();
        while (getline(inFile, line)) {
            if (line == "DATA") break;
            
            istringstream iss(line);
            string colName, colType;
            unsigned int constraints;
            
            if (!(iss >> colName >> colType >> constraints)) {
                cout << "Error: Invalid column format." << endl;
                return false;
            }
            
            columns.push_back(Column(colName, colType, constraints));
        }

        clearRows();
        while (getline(inFile, line)) {
            if (line.empty()) continue;
            
            istringstream iss(line);
            vector<string> values;
            string value;
            
            while (iss >> value) {
                values.push_back(value);
            }
            
            if (!values.empty()) {
                Row* newRow = new Row(values);
                rows.push_back(newRow);
            }
        }

        return true;
    }
};

class Database {
private:
    vector<Table> tables;

public:
    Database() {}

    ~Database() {
    }

    Table* getTable(const string& tableName) {
        for (Table& table : tables) {
            if (table.tableName == tableName) {
                return &table;
            }
        }
        return nullptr;
    }

    bool createTable(const string& tableName) {
        if (getTable(tableName) != nullptr) {
            cout << "Error: Table '" << tableName << "' already exists." << endl;
            return false;
        }
        
        Table newTable(tableName);
        tables.push_back(newTable);
        return true;
    }

    bool addColumnToTable(const string& tableName, const Column& column) {
        Table* table = getTable(tableName);
        if (table == nullptr) {
            cout << "Error: Table '" << tableName << "' not found." << endl;
            return false;
        }
        
        if (table->getColumnIndex(column.name) != -1) {
            cout << "Error: Column '" << column.name << "' already exists in table." << endl;
            return false;
        }
        
        table->addColumn(column);
        return true;
    }

    bool insertInto(const string& tableName, const vector<string>& values) {
        Table* table = getTable(tableName);
        if (table == nullptr) {
            cout << "Error: Table '" << tableName << "' not found." << endl;
            return false;
        }
        
        return table->insertRow(values);
    }

    void selectAll(const string& tableName) const {
        for (const Table& table : tables) {
            if (table.tableName == tableName) {
                table.displayAll();
                return;
            }
        }
        cout << "Error: Table '" << tableName << "' not found." << endl;
    }

    bool saveToFile(const string& filename) const {
        ofstream outFile(filename);
        if (!outFile.is_open()) {
            cout << "Error: Could not open file for writing." << endl;
            return false;
        }

        for (const Table& table : tables) {
            table.saveToFile(outFile);
            outFile << endl;
        }

        outFile.close();
        cout << "Database saved successfully to '" << filename << "'." << endl;
        return true;
    }

    bool loadFromFile(const string& filename) {
        ifstream inFile(filename);
        if (!inFile.is_open()) {
            cout << "Error: Could not open file for reading." << endl;
            return false;
        }

        tables.clear();
        string line;

        while (getline(inFile, line)) {
            if (line.empty()) continue;
            
            if (line.substr(0, 6) == "TABLE ") {
                Table tempTable;
                
                while (getline(inFile, line)) {
                    if (line == "DATA") break;
                    
                    istringstream iss(line);
                    string colName, colType;
                    unsigned int constraints;
                    
                    if (!(iss >> colName >> colType >> constraints)) break;
                    
                    tempTable.addColumn(Column(colName, colType, constraints));
                }

                while (getline(inFile, line)) {
                    if (line.empty() || line.substr(0, 6) == "TABLE ") {
                        if (!tempTable.tableName.empty() || !tempTable.columns.empty()) {
                            tables.push_back(tempTable);
                        }
                        
                        if (line.substr(0, 6) == "TABLE ") {
                            tempTable = Table();
                            tempTable.tableName = line.substr(6);
                        }
                        break;
                    }
                    
                    istringstream iss(line);
                    vector<string> values;
                    string value;
                    
                    while (iss >> value) {
                        values.push_back(value);
                    }
                    
                    if (!values.empty()) {
                        Row* newRow = new Row(values);
                        tempTable.rows.push_back(newRow);
                    }
                }
            }
        }

        if (!tables.empty() || !tables.back().tableName.empty()) {
        }

        inFile.close();
        cout << "Database loaded successfully from '" << filename << "'." << endl;
        return true;
    }

    void showTables() const {
        if (tables.empty()) {
            cout << "No tables in database." << endl;
            return;
        }
        
        cout << "Tables in database:" << endl;
        for (const Table& table : tables) {
            cout << "  - " << table.tableName << " (" << table.columns.size() 
                 << " columns, " << table.rows.size() << " rows)" << endl;
        }
    }
};

class CommandParser {
private:
    char buffer[256];  
public:
    CommandParser() {
        buffer[0] = '\0';
    }

    bool parseCreateTable(const string& input, string& tableName) {
        if (input.substr(0, 13) == "CREATE TABLE ") {
            tableName = input.substr(13);
            // Trim whitespace
            tableName.erase(tableName.find_last_not_of(" \n\r\t") + 1);
            tableName.erase(0, tableName.find_first_not_of(" \n\r\t"));
            return !tableName.empty();
        }
        return false;
    }

    bool parseAddColumn(const string& input, string& tableName, string& colName, 
                        string& colType, unsigned int& constraints) {
        if (input.substr(0, 11) == "ADD COLUMN ") {
            istringstream iss(input.substr(11));
            iss >> tableName >> colName >> colType >> constraints;
            return !tableName.empty() && !colName.empty() && !colType.empty();
        }
        return false;
    }

    bool parseInsert(const string& input, string& tableName, vector<string>& values) {
        if (input.substr(0, 12) == "INSERT INTO ") {
            size_t valuesPos = input.find(" VALUES");
            if (valuesPos != string::npos) {
                tableName = input.substr(12, valuesPos - 12);
                tableName.erase(tableName.find_last_not_of(" \n\r\t") + 1);
                
                string valuesStr = input.substr(valuesPos + 8);
                valuesStr.erase(remove(valuesStr.begin(), valuesStr.end(), '('), valuesStr.end());
                valuesStr.erase(remove(valuesStr.begin(), valuesStr.end(), ')'), valuesStr.end());
                
                istringstream iss(valuesStr);
                string value;
                while (iss >> value) {
                    values.push_back(value);
                }
                return !tableName.empty() && !values.empty();
            }
        }
        return false;
    }

    bool parseSelect(const string& input, string& tableName) {
        if (input.substr(0, 7) == "SELECT ") {
            size_t fromPos = input.find(" FROM ");
            if (fromPos != string::npos) {
                string selectPart = input.substr(7, fromPos - 7);
                if (selectPart == "*") {
                    tableName = input.substr(fromPos + 6);
                    tableName.erase(tableName.find_last_not_of(" \n\r\t") + 1);
                    return !tableName.empty();
                }
            }
        }
        return false;
    }

    bool parseSelectWhere(const string& input, string& tableName, 
                          string& colName, string& colValue) {
        if (input.substr(0, 7) == "SELECT ") {
            size_t fromPos = input.find(" FROM ");
            if (fromPos != string::npos) {
                string selectPart = input.substr(7, fromPos - 7);
                if (selectPart == "*") {
                    size_t wherePos = input.find(" WHERE ");
                    if (wherePos != string::npos) {
                        tableName = input.substr(fromPos + 6, wherePos - fromPos - 6);
                        tableName.erase(tableName.find_last_not_of(" \n\r\t") + 1);
                        
                        istringstream iss(input.substr(wherePos + 7));
                        iss >> colName >> colValue;
                        
                        return !tableName.empty() && !colName.empty() && !colValue.empty();
                    }
                }
            }
        }
        return false;
    }

    bool parseDelete(const string& input, string& tableName, 
                     string& colName, string& colValue) {
        if (input.substr(0, 12) == "DELETE FROM ") {
            size_t wherePos = input.find(" WHERE ");
            if (wherePos != string::npos) {
                tableName = input.substr(12, wherePos - 12);
                tableName.erase(tableName.find_last_not_of(" \n\r\t") + 1);
                
                istringstream iss(input.substr(wherePos + 7));
                iss >> colName >> colValue;
                
                return !tableName.empty() && !colName.empty() && !colValue.empty();
            }
        }
        return false;
    }

    bool parseSave(const string& input, string& filename) {
        if (input.substr(0, 5) == "SAVE ") {
            filename = input.substr(5);
            filename.erase(filename.find_last_not_of(" \n\r\t") + 1);
            return !filename.empty();
        }
        return false;
    }

    bool parseLoad(const string& input, string& filename) {
        if (input.substr(0, 5) == "LOAD ") {
            filename = input.substr(5);
            filename.erase(filename.find_last_not_of(" \n\r\t") + 1);
            return !filename.empty();
        }
        return false;
    }

    bool parseDropTable(const string& input, string& tableName) {
        if (input.substr(0, 11) == "DROP TABLE ") {
            tableName = input.substr(11);
            tableName.erase(tableName.find_last_not_of(" \n\r\t") + 1);
            return !tableName.empty();
        }
        return false;
    }
};

int main() {
    Database db;
    CommandParser parser;
    
    cout << "==========================================" << endl;
    cout << "     MINI DATABASE ENGINE v1.0            " << endl;
    cout << "==========================================" << endl;
    cout << endl;
    cout << "Commands:" << endl;
    cout << "  CREATE TABLE <name>                   - Create a new table" << endl;
    cout << "  ADD COLUMN <table> <col> <type> <cons> - Add column to table" << endl;
    cout << "  INSERT INTO <table> VALUES (v1, v2..) - Insert a row" << endl;
    cout << "  SELECT * FROM <table>                 - Display all records" << endl;
    cout << "  SELECT * FROM <table> WHERE <col> <val> - Select with condition" << endl;
    cout << "  DELETE FROM <table> WHERE <col> <val> - Delete matching rows" << endl;
    cout << "  DROP TABLE <name>                     - Delete a table" << endl;
    cout << "  SAVE <filename>                       - Save to file" << endl;
    cout << "  LOAD <filename>                       - Load from file" << endl;
    cout << "  SHOW TABLES                            - List all tables" << endl;
    cout << "  HELP                                   - Show this help" << endl;
    cout << "  EXIT                                   - Exit program" << endl;
    cout << endl;
    cout << "Constraint flags: 1=PK, 2=NOT NULL, 4=UNIQUE" << endl;
    cout << "  Example: PK+NOT NULL = 3 (1|2)" << endl;
    cout << "==========================================" << endl;
    cout << endl;

    string input;
    
    while (true) {
        cout << "db> ";
        getline(cin, input);
        
        char buffer[256];
        strncpy(buffer, input.c_str(), 255);
        buffer[255] = '\0';
        
        if (input.empty()) {
            continue;
        }
        
        string upperInput = input;
        transform(upperInput.begin(), upperInput.end(), upperInput.begin(), ::toupper);
        
        if (upperInput == "EXIT") {
            cout << "Exiting database. Goodbye!" << endl;
            break;
        }
        
        if (upperInput == "HELP") {
            cout << "Commands:" << endl;
            cout << "  CREATE TABLE <name>                   - Create a new table" << endl;
            cout << "  ADD COLUMN <table> <col> <type> <cons> - Add column to table" << endl;
            cout << "  INSERT INTO <table> VALUES (v1, v2..) - Insert a row" << endl;
            cout << "  SELECT * FROM <table>                 - Display all records" << endl;
            cout << "  SELECT * FROM <table> WHERE <col> <val> - Select with condition" << endl;
            cout << "  DELETE FROM <table> WHERE <col> <val> - Delete matching rows" << endl;
            cout << "  DROP TABLE <name>                     - Delete a table" << endl;
            cout << "  SAVE <filename>                       - Save to file" << endl;
            cout << "  LOAD <filename>                       - Load from file" << endl;
            cout << "  SHOW TABLES                            - List all tables" << endl;
            cout << "  HELP                                   - Show this help" << endl;
            cout << "  EXIT                                   - Exit program" << endl;
            cout << endl;
            cout << "Constraint flags: 1=PK, 2=NOT NULL, 4=UNIQUE" << endl;
            cout << "  Example: PK+NOT NULL = 3 (1|2)" << endl;
            continue;
        }
        
        if (upperInput == "SHOW TABLES") {
            db.showTables();
            continue;
        }

        string tableName;
        if (parser.parseCreateTable(input, tableName)) {
            if (db.createTable(tableName)) {
                cout << "Table '" << tableName << "' created successfully." << endl;
            }
            continue;
        }

        string colName, colType;
        unsigned int constraints;
        if (parser.parseAddColumn(input, tableName, colName, colType, constraints)) {
            Column column(colName, colType, constraints);
            string constraintStr;
            if (column.isPrimaryKey()) constraintStr += "PRIMARY KEY ";
            if (column.isNotNull()) constraintStr += "NOT NULL ";
            if (column.isUnique()) constraintStr += "UNIQUE ";
            
            if (db.addColumnToTable(tableName, column)) {
                cout << "Column '" << colName << "' added to table '" << tableName << "'." << endl;
                if (!constraintStr.empty()) {
                    cout << "  Constraints: " << constraintStr << endl;
                }
            }
            continue;
        }

        vector<string> values;
        if (parser.parseInsert(input, tableName, values)) {
            if (db.insertInto(tableName, values)) {
                cout << "Record inserted into '" << tableName << "'." << endl;
            }
            continue;
        }

        if (parser.parseSelect(input, tableName)) {
            db.selectAll(tableName);
            continue;
        }

        string colValue;
        if (parser.parseSelectWhere(input, tableName, colName, colValue)) {
            Table* table = db.getTable(tableName);
            if (table) {
                vector<Row*> results = table->selectWhere(colName, colValue);
                if (results.empty()) {
                    cout << "No records found." << endl;
                } else {
                    for (int i = 0; i < (int)table->columns.size(); i++) {
                        cout << table->columns[i].name << "\t";
                    }
                    cout << endl;
                    
                    for (int i = 0; i < (int)table->columns.size(); i++) {
                        cout << "----";
                    }
                    cout << endl;
                    
                    for (Row* row : results) {
                        for (int i = 0; i < (int)row->values.size(); i++) {
                            cout << row->values[i] << "\t";
                        }
                        cout << endl;
                    }
                    cout << results.size() << " record(s) found." << endl;
                }
            }
            continue;
        }

        if (parser.parseDelete(input, tableName, colName, colValue)) {
            Table* table = db.getTable(tableName);
            if (table) {
                int count = table->deleteWhere(colName, colValue);
                cout << count << " record(s) deleted from '" << tableName << "'." << endl;
            }
            continue;
        }

        if (parser.parseDropTable(input, tableName)) {
            Table* table = db.getTable(tableName);
            if (table) {
              cout << "Table '" << tableName << "' dropped." << endl;
            } else {
                cout << "Error: Table '" << tableName << "' not found." << endl;
            }
            continue;
        }

        string filename;
        if (parser.parseSave(input, filename)) {
            db.saveToFile(filename);
            continue;
        }

        if (parser.parseLoad(input, filename)) {
            db.loadFromFile(filename);
            continue;
        }

        cout << "Error: Unknown command. Type HELP for available commands." << endl;
    }

    return 0;
}
