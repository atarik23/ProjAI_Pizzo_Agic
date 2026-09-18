#include "Loader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

static std::runtime_error parseErr(const std::string& src, int line, const std::string& msg) {
    std::ostringstream oss;
    oss << "Loader error in " << src << ":" << line << " - " << msg;
    return std::runtime_error(oss.str());
}

static inline void trimInPlace(std::string& s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
}

static inline void stripUTF8BOM(std::string& s) {
    if (s.size() >= 3 &&
        (unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF) {
        s.erase(0, 3);
    }
}

static LoadedNeighborhood parseNeighborhoodStream(std::istream& in, const std::string& srcName) {
    LoadedNeighborhood out;
    bool hasV = false;

    std::string line;
    int lineNo = 0;
    bool firstLine = true;

    while (std::getline(in, line)) {
        lineNo++;

        if (!line.empty() && line.back() == '\r') line.pop_back();

        if (firstLine) {
            stripUTF8BOM(line);
            firstLine = false;
        }

        trimInPlace(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::istringstream iss(line);
        std::string tag;
        iss >> tag;

        if (tag == "V") {
            int n = 0;
            if (!(iss >> n)) throw parseErr(srcName, lineNo, "Expected: V <number>");
            if (n <= 0) throw parseErr(srcName, lineNo, "V must be > 0");

            out.graph.init(n);
            out.nodes.clear();
            hasV = true;
        }
        else if (tag == "LABELS") {
            if (!hasV) throw parseErr(srcName, lineNo, "LABELS before V");

            out.nodes.clear();
            out.nodes.reserve(out.graph.vertexCount());

            std::string lbl;
            while (iss >> lbl) out.nodes.push_back(lbl);

            if (!out.nodes.empty() && (int)out.nodes.size() != out.graph.vertexCount()) {
                throw parseErr(srcName, lineNo, "LABELS count must match V");
            }
        }
        else if (tag == "E") {
            if (!hasV) throw parseErr(srcName, lineNo, "Edge before V");

            int u, v, cost;
            if (!(iss >> u >> v >> cost)) {
                throw parseErr(srcName, lineNo, "Expected: E <u> <v> <cost>");
            }

            if (u < 0 || v < 0 || u >= out.graph.vertexCount() || v >= out.graph.vertexCount())
                throw parseErr(srcName, lineNo, "Edge endpoint out of range");
            if (cost <= 0)
                throw parseErr(srcName, lineNo, "Edge cost must be positive");

            out.graph.addEdge(u, v, cost);
        }
        else {
            throw parseErr(srcName, lineNo, "Unknown line tag: '" + tag + "' | line='" + line + "'");
        }
    }

    if (!hasV) throw std::runtime_error("Loader: missing V line in: " + srcName);
    if (out.graph.edgeCount() == 0) throw std::runtime_error("Loader: no edges in: " + srcName);

    return out;
}

LoadedNeighborhood Loader::loadNeighborhood(const std::string& filePath) {
    std::ifstream in(filePath);
    if (!in.is_open()) {
        throw std::runtime_error("Loader: cannot open file: " + filePath);
    }
    return parseNeighborhoodStream(in, filePath);
}

LoadedNeighborhood Loader::loadNeighborhoodFromString(const std::string& text, const std::string& virtualName) {
    std::istringstream in(text);
    return parseNeighborhoodStream(in, virtualName);
}