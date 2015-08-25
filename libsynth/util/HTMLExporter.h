
class CategoryDistribution;

struct HTMLExporter
{
    HTMLExporter(const string &filename, const string &headerText);
    ~HTMLExporter();
    
    void appendLineBreak();
    void appendHeader1(const string &header);
    void appendHeader2(const string &header);
    void appendHeader3(const string &header);
    void appendText(const string &text);
    void appendBoldText(const string &header, const string &text);

    void appendTableColumnWrap(const string &tableName, const ml::Grid2<string> &entries, unsigned int columnCount);
    void appendTable(const string &tableName, const ml::Grid2<string> &entries);
    void appendTable(const string &tableName, const vector<string> &headers, const vector<string> &entries);
    void appendTable(const string &tableName, const vector<string> &headers, const ml::Grid2<string> &entries);
    
    std::ofstream file;
};
