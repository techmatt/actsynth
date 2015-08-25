
#include "libsynth.h"

HTMLExporter::HTMLExporter(const string &filename, const string &headerText)
{
    file.open(filename);
    file << "<html><head></head><body>" << endl;
    file << "<h1>" << headerText << "</h1><br/>" << endl;
}

HTMLExporter::~HTMLExporter()
{
    file << "</body></html>" << endl;
    file.close();
}

void HTMLExporter::appendLineBreak()
{
    file << "<br/>" << endl;
}

void HTMLExporter::appendHeader1(const string &header)
{
    file << "<h1>" << header << "</h1>" << endl;
}

void HTMLExporter::appendHeader2(const string &header)
{
    file << "<h2>" << header << "</h2>" << endl;
}

void HTMLExporter::appendHeader3(const string &header)
{
    file << "<h3>" << header << "</h3>" << endl;
}

void HTMLExporter::appendText(const string &text)
{
    file << "<p>" << text << "</p>" << endl;
}

void HTMLExporter::appendBoldText(const string &header, const string &text)
{
    file << "<p><b>" << header << "</b>" << text << "</p>" << endl;
}

void HTMLExporter::appendTable(const string &tableName, const vector<string> &headers, const vector<string> &entries)
{
    const size_t cols = headers.size();
    ml::Grid2<string> entriesGrid(entries.size() / cols, cols);

    int entryIndex = 0;
    for (auto &e : entriesGrid)
        e.value = entries[entryIndex++];

    appendTable(tableName, headers, entriesGrid);
}

void HTMLExporter::appendTableColumnWrap(const string &tableName, const ml::Grid2<string> &entries, unsigned int columnCount)
{
    if (tableName.size() > 0)
        appendHeader1(tableName);

    int activeX = 0;
    while (activeX < entries.getDimX())
    {
        ml::Grid2<string> g(columnCount, entries.getDimY());

        for (unsigned int x = 0; x < columnCount; x++)
        {
            if (activeX < entries.getDimX())
                for (unsigned int y = 0; y < g.getDimY(); y++)
                    g(x, y) = entries(activeX, y);
            activeX++;
        }

        appendTable("", g);
    }
}

void HTMLExporter::appendTable(const string &tableName, const ml::Grid2<string> &entries)
{
    vector<string> headers;
    appendTable(tableName, headers, entries);
}

void HTMLExporter::appendTable(const string &tableName, const vector<string> &headers, const ml::Grid2<string> &entries)
{
    if(tableName.size() > 0)
        appendHeader1(tableName);
    
    file << "<table>" << endl;

    if (headers.size() > 0)
    {
        file << "  <tr>";
        for (const string &s : headers)
        {
            file << "<td><b>" << s << "</b></td>";
        }
        file << "</tr>" << endl;
    }
    

    for (UINT y = 0; y < entries.getDimY(); y++)
    {
        file << "  <tr>";
        for (const string &s : entries.getRow(y))
        {
            file << "<td>" << s << "</td>";
        }
        file << "</tr>" << endl;
    }

    file << "</table><br/>" << endl;
}
