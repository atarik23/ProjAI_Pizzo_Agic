#include "Route.h"
#include <sstream>

long long Route::totalCost() const {
    long long s = 0;
    for (const auto& st : steps) s += st.cost;
    return s;
}

std::string Route::toString(const std::vector<std::string>* labels) const {
    std::ostringstream oss;

    for (const auto& st : steps) {
        auto printV = [&](int v) {
            if (labels && !labels->empty()) oss << (*labels)[v];
            else oss << v;
            };

        printV(st.from);
        oss << " -> ";
        printV(st.to);
        oss << " (cost=" << st.cost << ")\n";
    }

    return oss.str();
}
