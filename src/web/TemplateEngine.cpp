#include "web/TemplateEngine.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "core/Utils.h"

namespace web {

// ------------------------------------------------------------------ Context
Context& Context::set(const std::string& key, const std::string& value) {
    values[key] = value;
    return *this;
}

Context& Context::set(const std::string& key, const char* value) { return set(key, std::string(value ? value : "")); }

Context& Context::set(const std::string& key, int value) { return set(key, cab::utils::toString(value)); }

Context& Context::set(const std::string& key, double value) { return set(key, cab::utils::fixed(value, 2)); }

std::string Context::get(const std::string& key) const {
    auto it = values.find(key);
    return it == values.end() ? "" : it->second;
}

// ------------------------------------------------------------------ TemplateEngine
TemplateEngine::TemplateEngine(const std::string& directory) : directory(directory) {}

std::string TemplateEngine::load(const std::string& name) const {
    // Read from disk each time so templates can be edited without recompiling.
    std::ifstream file((directory + "/" + name).c_str(), std::ios::binary);
    if (!file) throw std::runtime_error("Template not found: " + name);
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

std::string TemplateEngine::render(const std::string& name, const Context& context) const {
    const std::string source = load(name);
    std::string out;
    out.reserve(source.size() * 2);

    std::string::size_type pos = 0;
    while (pos < source.size()) {
        std::string::size_type open = source.find("{{", pos);
        if (open == std::string::npos) {
            out.append(source, pos, std::string::npos);
            break;
        }
        out.append(source, pos, open - pos);

        bool raw = source.compare(open, 3, "{{{") == 0;
        const char* closer = raw ? "}}}" : "}}";
        std::string::size_type start = open + (raw ? 3 : 2);
        std::string::size_type close = source.find(closer, start);
        if (close == std::string::npos) {
            out.append(source, open, std::string::npos);
            break;
        }
        std::string key = cab::utils::trim(source.substr(start, close - start));
        std::string value = context.get(key);
        out += raw ? value : cab::utils::htmlEscape(value);
        pos = close + (raw ? 3 : 2);
    }
    return out;
}

}  // namespace web
