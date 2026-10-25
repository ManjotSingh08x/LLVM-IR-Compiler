#ifndef SEMANTIC_DIAGNOSTICS_H
#define SEMANTIC_DIAGNOSTICS_H

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

namespace sema {

//semantic errors, printed in the parser's syntax error format
class Diagnostics {
public:
    void error(int line, int col, const std::string &message) {
        errors.push_back({ line, col, message });
    }

    bool   empty() const { return errors.empty(); }
    size_t count() const { return errors.size(); }

    //in source order, since the passes do not visit the file front to back
    void print(std::ostream &out, const char *filename) {
        std::stable_sort(errors.begin(), errors.end(), [](const Error &a, const Error &b) {
            return a.line != b.line ? a.line < b.line : a.col < b.col;
        });
        for (const Error &e : errors) {
            out << filename << " line: " << e.line << " col: " << e.col
                << " semantic error: " << e.message << "\n";
        }
        out << errors.size() << " semantic error(s) found." << std::endl;
    }

private:
    struct Error {
        int         line;
        int         col;
        std::string message;
    };

    std::vector<Error> errors;
};

} // namespace sema

#endif
